//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-21 04:31:42
//

#include <Renderer/Techniques/GBuffer.hpp>

#include <Settings.hpp>
#include <Statistics.hpp>

GBuffer::GBuffer(RHI::Ref rhi)
    : RenderPass(rhi)
{ 
    mSampler = mRHI->CreateSampler(SamplerAddress::Wrap, SamplerFilter::Linear, true);

    GraphicsPipelineSpecs specs;
    specs.Fill = FillMode::Solid;
    specs.Cull = CullMode::None;
    specs.Depth = DepthOperation::Less;
    specs.CCW = false;
    specs.DepthEnabled = true;
    specs.DepthFormat = TextureFormat::Depth32;
    specs.Signature = mRHI->CreateRootSignature({ RootType::PushConstant }, sizeof(int) * 4);
    
    mPipeline.Init(rhi, specs);
    mPipeline.AddPermutation("NoAlpha", "Assets/Shaders/GBuffer/Vertex.hlsl", "Assets/Shaders/GBuffer/FragmentNoAlpha.hlsl");
    mPipeline.AddPermutation("Alpha", "Assets/Shaders/GBuffer/Vertex.hlsl", "Assets/Shaders/GBuffer/FragmentAlpha.hlsl");
}

void GBuffer::Render(const Frame& frame, Scene& scene)
{
    ::Ref<RenderPassIO> white = PassManager::Get("WhiteTexture");
    ::Ref<RenderPassIO> depth = PassManager::Get("GBufferDepth");
    ::Ref<RenderPassIO> camera = PassManager::Get("CameraRingBuffer");

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

    frame.CommandBuffer->BeginMarker("GBuffer");
    frame.CommandBuffer->Barrier(depth->Texture, ResourceLayout::DepthWrite);
    frame.CommandBuffer->ClearDepth(depth->DepthTargetView);
    frame.CommandBuffer->SetRenderTargets({}, depth->DepthTargetView);
    frame.CommandBuffer->SetTopology(Topology::TriangleList);
    frame.CommandBuffer->SetViewport(0, 0, (float)frame.Width, (float)frame.Height);

    std::function<void(Frame frame, GLTFNode*, GLTF* model, glm::mat4 transform)> drawNode = [&](Frame frame, GLTFNode* node, GLTF* model, glm::mat4 transform) {
        if (!node) {
            return;
        }

        glm::mat4 globalTransform = transform * node->Transform;
        glm::mat4 invTransform = glm::inverse(globalTransform);
        for (GLTFPrimitive primitive : node->Primitives) {
            if (!scene.Camera.IsBoxInFrustum(primitive.AABB, globalTransform) && Settings::Get().FrustumCull) {
                Statistics::Get().CulledInstances++;
                Statistics::Get().CulledTriangles += primitive.IndexCount / 3;
                continue;
            }
            Statistics::Get().InstanceCount++;

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
                int TextureIndex;          
                int SamplerIndex;
            } Constants = {
                camera->RingBuffer[frame.FrameIndex]->CBV(),
                node->ModelBuffer[frame.FrameIndex]->CBV(),
                albedoIndex,
                mSampler->BindlesssSampler(),
            };
            frame.CommandBuffer->SetGraphicsPipeline(material.AlphaTested ? mPipeline.Get("Alpha") : mPipeline.Get("NoAlpha"));
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

void GBuffer::UI(const Frame& frame)
{

}
