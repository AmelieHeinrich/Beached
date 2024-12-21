//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-21 04:31:42
//

#include <Renderer/Techniques/GBuffer.hpp>

GBuffer::GBuffer(RHI::Ref rhi)
    : RenderPass(rhi)
{
    ::Ref<RenderPassIO> normal = PassManager::Get("GBufferNormal");
    ::Ref<RenderPassIO> albedo = PassManager::Get("GBufferAlbedo");

    Asset::Handle vertexShader = AssetManager::Get("Assets/Shaders/GBuffer/Vertex.hlsl", AssetType::Shader);
    Asset::Handle fragmentShader = AssetManager::Get("Assets/Shaders/GBuffer/Fragment.hlsl", AssetType::Shader);
    
    GraphicsPipelineSpecs specs;
    specs.Fill = FillMode::Solid;
    specs.Cull = CullMode::None;
    specs.Depth = DepthOperation::Less;
    specs.CCW = false;
    specs.DepthEnabled = true;
    specs.DepthFormat = TextureFormat::Depth32;
    specs.Formats.push_back(normal->Desc.Format);
    specs.Formats.push_back(albedo->Desc.Format);
    specs.Bytecodes[ShaderType::Vertex] = vertexShader->Shader;
    specs.Bytecodes[ShaderType::Fragment] = fragmentShader->Shader;
    specs.Signature = mRHI->CreateRootSignature({ RootType::PushConstant }, sizeof(int) * 4);
    
    mPipeline = mRHI->CreateGraphicsPipeline(specs);
    mSampler = mRHI->CreateSampler(SamplerAddress::Wrap, SamplerFilter::Linear, true);
}

void GBuffer::Render(const Frame& frame, const Scene& scene)
{
    ::Ref<RenderPassIO> white = PassManager::Get("WhiteTexture");
    ::Ref<RenderPassIO> depth = PassManager::Get("GBufferDepth");
    ::Ref<RenderPassIO> normal = PassManager::Get("GBufferNormal");
    ::Ref<RenderPassIO> albedo = PassManager::Get("GBufferAlbedo");
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
    frame.CommandBuffer->Barrier(normal->Texture, ResourceLayout::ColorWrite);
    frame.CommandBuffer->Barrier(albedo->Texture, ResourceLayout::ColorWrite);
    frame.CommandBuffer->Barrier(depth->Texture, ResourceLayout::DepthWrite);
    frame.CommandBuffer->ClearRenderTarget(albedo->RenderTargetView, 0.0f, 0.0f, 0.0f);
    frame.CommandBuffer->ClearRenderTarget(normal->RenderTargetView, 0.0f, 0.0f, 0.0f);
    frame.CommandBuffer->ClearDepth(depth->DepthTargetView);
    frame.CommandBuffer->SetRenderTargets({ normal->RenderTargetView, albedo->RenderTargetView }, depth->DepthTargetView);
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
                int TextureIndex;          
                int SamplerIndex;
            } Constants = {
                camera->RingBuffer[frame.FrameIndex]->CBV(),
                node->ModelBuffer[frame.FrameIndex]->CBV(),
                albedoIndex,
                mSampler->BindlesssSampler(),
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

void GBuffer::UI(const Frame& frame)
{

}
