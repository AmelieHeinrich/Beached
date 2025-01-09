//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-06 00:14:09
//

#include <Renderer/Techniques/Forward.hpp>
#include <Renderer/Techniques/CSM.hpp>
#include <Renderer/Techniques/Debug.hpp>

#include <Settings.hpp>

#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>

Forward::Forward(RHI::Ref rhi)
    : RenderPass(rhi)
{
    mSampler = mRHI->CreateSampler(SamplerAddress::Wrap, SamplerFilter::Linear, true);
    mClampSampler = mRHI->CreateSampler(SamplerAddress::Clamp, SamplerFilter::Nearest, false);
    mShadowSampler = mRHI->CreateSampler(SamplerAddress::Clamp, SamplerFilter::Nearest, false, 1, true);

    ::Ref<RenderPassIO> color = PassManager::Get("MainColorBuffer");

    GraphicsPipelineSpecs specs;
    specs.Fill = FillMode::Solid;
    specs.Cull = CullMode::None;
    specs.Depth = DepthOperation::Equal;
    specs.CCW = false;
    specs.DepthEnabled = true;
    specs.DepthFormat = TextureFormat::Depth32;
    specs.Formats.push_back(color->Desc.Format);
    specs.Signature = mRHI->CreateRootSignature({ RootType::PushConstant }, sizeof(int) * 10);
    
    mPipeline.Init(rhi, specs);
    mPipeline.AddPermutation("Alpha", "Assets/Shaders/Forward/Vertex.hlsl", "Assets/Shaders/Forward/FragmentAlpha.hlsl");
    mPipeline.AddPermutation("NoAlpha", "Assets/Shaders/Forward/Vertex.hlsl", "Assets/Shaders/Forward/FragmentNoAlpha.hlsl");
}

void Forward::Render(const Frame& frame, Scene& scene)
{
    ::Ref<RenderPassIO> color = PassManager::Get("MainColorBuffer");
    ::Ref<RenderPassIO> depth = PassManager::Get("GBufferDepth");
    ::Ref<RenderPassIO> camera = PassManager::Get("CameraRingBuffer");
    ::Ref<RenderPassIO> white = PassManager::Get("WhiteTexture");
    ::Ref<RenderPassIO> cascade = PassManager::Get("CascadeRingBuffer");

    mCulledOBBs = 0;

    struct UploadData {
        glm::mat4 View;
        glm::mat4 Projection;
        glm::mat4 InvView;
        glm::vec3 Position;
        float Pad;
    } Data = {
        scene.Camera.View(),
        scene.Camera.Projection(),
        glm::inverse(scene.Camera.View()),
        scene.Camera.Position(),
        0.0f,
    };
    camera->RingBuffer[frame.FrameIndex]->CopyMapped(&Data, sizeof(Data));

    frame.CommandBuffer->BeginMarker("Forward");
    frame.CommandBuffer->Barrier(color->Texture, ResourceLayout::ColorWrite);
    frame.CommandBuffer->Barrier(depth->Texture, ResourceLayout::DepthWrite);
    frame.CommandBuffer->ClearRenderTarget(color->RenderTargetView, 0.1f, 0.1f, 0.1f);
    frame.CommandBuffer->SetRenderTargets({ color->RenderTargetView }, depth->DepthTargetView);
    frame.CommandBuffer->SetTopology(Topology::TriangleList);
    frame.CommandBuffer->SetViewport(0, 0, (float)frame.Width, (float)frame.Height);

    std::function<void(Frame frame, GLTFNode*, GLTF* model, glm::mat4 transform)> drawNode = [&](Frame frame, GLTFNode* node, GLTF* model, glm::mat4 transform) {
        if (!node) {
            return;
        }

        glm::mat4 globalTransform = transform * node->Transform;
        glm::mat4 invTransform = glm::inverse(globalTransform);
        for (GLTFPrimitive primitive : node->Primitives) {
            // CPU cull
            if (!scene.Camera.IsBoxInFrustum(primitive.AABB, globalTransform) && Settings::Get().FrustumCull) {
                mCulledOBBs++;
                continue;
            }

            GLTFMaterial material = model->Materials[primitive.MaterialIndex];
            glm::mat4 matrices[] = {
                globalTransform,
                invTransform
            };
            node->ModelBuffer[frame.FrameIndex]->CopyMapped(matrices, sizeof(glm::mat4) * 2);
            
            int albedoIndex = material.Albedo ? material.AlbedoView->GetDescriptor().Index : white->ShaderResourceView->GetDescriptor().Index;
            int normalIndex = material.Normal ? material.NormalView->GetDescriptor().Index : -1;

            struct PushConstants {
                int CameraIndex;
                int ModelIndex;
                int LightIndex;
                int CascadeIndex;

                int TextureIndex;
                int NormalIndex;

                int SamplerIndex;
                int ClampSamplerIndex;
                int ShadowSamplerIndex;

                int Accel;
            } Constants = {
                camera->RingBuffer[frame.FrameIndex]->CBV(),
                node->ModelBuffer[frame.FrameIndex]->CBV(),
                scene.LightBuffer[frame.FrameIndex]->CBV(),
                cascade->RingBuffer[frame.FrameIndex]->CBV(),

                albedoIndex,
                normalIndex,

                mSampler->BindlesssSampler(),
                mClampSampler->BindlesssSampler(),
                mShadowSampler->BindlesssSampler(),

                -1
            };
            frame.CommandBuffer->SetGraphicsPipeline(material.AlphaTested ? mPipeline.Get("Alpha") : mPipeline.Get("NoAlpha"));
            frame.CommandBuffer->GraphicsPushConstants(&Constants, sizeof(Constants), 0);
            frame.CommandBuffer->SetVertexBuffer(primitive.VertexBuffer);
            frame.CommandBuffer->SetIndexBuffer(primitive.IndexBuffer);
            frame.CommandBuffer->DrawIndexed(primitive.IndexCount);

            if (Settings::Get().DebugDrawVolumes) {
                Debug::DrawBox(globalTransform, primitive.AABB.Min, primitive.AABB.Max, glm::vec3(0.0f, 1.0, 0.0f));
            }
        }
        
        if (!node->Children.empty()) {
            for (GLTFNode* child : node->Children) {
                drawNode(frame, child, model, globalTransform);
            }
        }
    };
    for (auto& model : scene.Models) {
        drawNode(frame, model->Model.Root, &model->Model, glm::mat4(1.0f));
    }
    frame.CommandBuffer->EndMarker();
}

void Forward::UI(const Frame& frame)
{
    if (ImGui::TreeNodeEx("Forward", ImGuiTreeNodeFlags_Framed)) {
        ImGui::TreePop();
    }
}
