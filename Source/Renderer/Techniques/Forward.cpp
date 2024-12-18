//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-06 00:14:09
//

#include <Renderer/Techniques/Forward.hpp>
#include <Renderer/Techniques/CSM.hpp>

#include <glm/gtc/type_ptr.hpp>

Forward::Forward(RHI::Ref rhi)
    : RenderPass(rhi)
{
    ::Ref<RenderPassIO> color = PassManager::Get("MainColorBuffer");

    Asset::Handle vertexShader = AssetManager::Get("Assets/Shaders/Forward/Vertex.hlsl", AssetType::Shader);
    Asset::Handle fragmentShader = AssetManager::Get("Assets/Shaders/Forward/Fragment.hlsl", AssetType::Shader);
    
    GraphicsPipelineSpecs triangleSpecs;
    triangleSpecs.Fill = FillMode::Solid;
    triangleSpecs.Cull = CullMode::None;
    triangleSpecs.Depth = DepthOperation::Less;
    triangleSpecs.CCW = false;
    triangleSpecs.DepthEnabled = true;
    triangleSpecs.DepthFormat = TextureFormat::Depth32;
    triangleSpecs.Formats.push_back(color->Desc.Format);
    triangleSpecs.Bytecodes[ShaderType::Vertex] = vertexShader->Shader;
    triangleSpecs.Bytecodes[ShaderType::Fragment] = fragmentShader->Shader;
    triangleSpecs.Signature = mRHI->CreateRootSignature({ RootType::PushConstant }, sizeof(int) * 8);
    
    mPipeline = mRHI->CreateGraphicsPipeline(triangleSpecs);
    mSampler = mRHI->CreateSampler(SamplerAddress::Wrap, SamplerFilter::Linear, true);
    mShadowSampler = mRHI->CreateSampler(SamplerAddress::Clamp, SamplerFilter::Nearest, false);
}

void Forward::Render(const Frame& frame, const Scene& scene)
{
    ::Ref<RenderPassIO> color = PassManager::Get("MainColorBuffer");
    ::Ref<RenderPassIO> depth = PassManager::Get("MainDepthBuffer");
    ::Ref<RenderPassIO> camera = PassManager::Get("CameraRingBuffer");
    ::Ref<RenderPassIO> white = PassManager::Get("WhiteTexture");
    ::Ref<RenderPassIO> cascade = PassManager::Get("CascadeRingBuffer");

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
    frame.CommandBuffer->ClearDepth(depth->DepthTargetView);
    frame.CommandBuffer->SetRenderTargets({ color->RenderTargetView }, depth->DepthTargetView);
    frame.CommandBuffer->SetTopology(Topology::TriangleList);
    frame.CommandBuffer->SetViewport(0, 0, (float)frame.Width, (float)frame.Height);
    frame.CommandBuffer->SetGraphicsPipeline(mPipeline);

    std::function<void(Frame frame, GLTFNode*, GLTF* model, glm::mat4 transform)> drawNode = [&](Frame frame, GLTFNode* node, GLTF* model, glm::mat4 transform) {
        if (!node) {
            return;
        }

        glm::mat4 globalTransform = transform * node->Transform;
        glm::mat4 invTransform = glm::inverse(globalTransform);
        for (GLTFPrimitive primitive : node->Primitives) {
            GLTFMaterial material = model->Materials[primitive.MaterialIndex];

            glm::mat4 matrices[] = {
                globalTransform,
                invTransform
            };
            node->ModelBuffer[frame.FrameIndex]->CopyMapped(matrices, sizeof(glm::mat4) * 2);
            
            int albedoIndex = material.Albedo ? material.AlbedoView->GetDescriptor().Index : white->ShaderResourceView->GetDescriptor().Index;

            struct PushConstants {
                int CameraIndex;
                int ModelIndex;
                int LightIndex;
                int CascadeIndex;

                int TextureIndex;
                
                int SamplerIndex;
                int ShadowSamplerIndex;

                int Accel;
            } Constants = {
                camera->RingBuffer[frame.FrameIndex]->CBV(),
                node->ModelBuffer[frame.FrameIndex]->CBV(),
                scene.LightBuffer[frame.FrameIndex]->CBV(),
                cascade->RingBuffer[frame.FrameIndex]->CBV(),

                albedoIndex,

                mSampler->BindlesssSampler(),
                mShadowSampler->BindlesssSampler(),

                scene.TLAS->Bindless()
            };
            frame.CommandBuffer->GraphicsPushConstants(&Constants, sizeof(Constants), 0);
            frame.CommandBuffer->SetVertexBuffer(primitive.VertexBuffer);
            frame.CommandBuffer->SetIndexBuffer(primitive.IndexBuffer);
            frame.CommandBuffer->DrawIndexed(primitive.IndexCount);
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

}
