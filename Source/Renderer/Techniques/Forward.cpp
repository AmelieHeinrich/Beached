//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-06 00:14:09
//

#include <Renderer/Techniques/Forward.hpp>

#include <glm/gtc/type_ptr.hpp>

Forward::Forward(RHI::Ref rhi)
    : RenderPass(rhi)
{
    Asset::Handle vertexShader = AssetManager::Get("Assets/Shaders/Forward/Vertex.hlsl", AssetType::Shader);
    Asset::Handle fragmentShader = AssetManager::Get("Assets/Shaders/Forward/Fragment.hlsl", AssetType::Shader);
    
    GraphicsPipelineSpecs triangleSpecs;
    triangleSpecs.Fill = FillMode::Solid;
    triangleSpecs.Cull = CullMode::None;
    triangleSpecs.Depth = DepthOperation::Less;
    triangleSpecs.CCW = false;
    triangleSpecs.DepthEnabled = true;
    triangleSpecs.DepthFormat = TextureFormat::Depth32;
    triangleSpecs.Formats.push_back(TextureFormat::RGBA8);
    triangleSpecs.Bytecodes[ShaderType::Vertex] = vertexShader->Shader;
    triangleSpecs.Bytecodes[ShaderType::Fragment] = fragmentShader->Shader;
    triangleSpecs.Signature = mRHI->CreateRootSignature({ RootType::PushConstant }, sizeof(int) * 4);
    
    mPipeline = mRHI->CreateGraphicsPipeline(triangleSpecs);
    mSampler = mRHI->CreateSampler(SamplerAddress::Wrap, SamplerFilter::Linear, true);
}

void Forward::Render(const Frame& frame, const Scene& scene)
{
    ::Ref<RenderPassIO> color = PassManager::Get("MainColorBuffer");
    ::Ref<RenderPassIO> depth = PassManager::Get("MainDepthBuffer");
    ::Ref<RenderPassIO> camera = PassManager::Get("CameraRingBuffer");

    frame.CommandBuffer->Barrier(color->Texture, ResourceLayout::ColorWrite);
    frame.CommandBuffer->Barrier(depth->Texture, ResourceLayout::DepthWrite);
    frame.CommandBuffer->ClearRenderTarget(color->RenderTargetView, 0.1f, 0.1f, 0.1f);
    frame.CommandBuffer->ClearDepth(depth->DepthTargetView);
    frame.CommandBuffer->SetRenderTargets({ color->RenderTargetView }, depth->DepthTargetView);
    
    glm::mat4 uploads[] = {
        scene.Camera.View(),
        scene.Camera.Projection()
    };
    camera->RingBuffer[frame.FrameIndex]->CopyMapped(uploads, sizeof(uploads));

    std::function<void(Frame frame, GLTFNode*, GLTF* model, glm::mat4 transform)> drawNode = [&](Frame frame, GLTFNode* node, GLTF* model, glm::mat4 transform) {
        if (!node) {
            return;
        }

        glm::mat4 globalTransform = transform * node->Transform;
        for (GLTFPrimitive primitive : node->Primitives) {
            GLTFMaterial material = model->Materials[primitive.MaterialIndex];
            node->ModelBuffer[frame.FrameIndex]->CopyMapped(glm::value_ptr(globalTransform), sizeof(glm::mat4));
            
            int resources[] = {
                camera->RingBuffer[frame.FrameIndex]->CBV(),
                node->ModelBuffer[frame.FrameIndex]->CBV(),
                material.AlbedoView->GetDescriptor().Index,
                mSampler->BindlesssSampler()
            };
            frame.CommandBuffer->GraphicsPushConstants(resources, sizeof(resources), 0);
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

    frame.CommandBuffer->SetTopology(Topology::TriangleList);
    frame.CommandBuffer->SetViewport(0, 0, (float)frame.Width, (float)frame.Height);
    frame.CommandBuffer->SetGraphicsPipeline(mPipeline);
    for (auto& model : scene.Models) {
        drawNode(frame, model->Model.Root, &model->Model, glm::mat4(1.0f));
    }
}

void Forward::UI()
{

}
