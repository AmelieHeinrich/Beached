//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-07 02:19:53
//

#include <Renderer/Techniques/Debug.hpp>
#include <Core/Math.hpp>

Debug::Data Debug::sData;

Debug::Debug(RHI::Ref rhi)
    : RenderPass(rhi)
{
    Asset::Handle vertexShader = AssetManager::Get("Assets/Shaders/Debug/Vertex.hlsl", AssetType::Shader);
    Asset::Handle fragmentShader = AssetManager::Get("Assets/Shaders/Debug/Fragment.hlsl", AssetType::Shader);
    
    GraphicsPipelineSpecs specs;
    specs.Fill = FillMode::Solid;
    specs.Cull = CullMode::None;
    specs.CCW = false;
    specs.Line = true;
    specs.Formats.push_back(TextureFormat::RGBA8);
    specs.Bytecodes[ShaderType::Vertex] = vertexShader->Shader;
    specs.Bytecodes[ShaderType::Fragment] = fragmentShader->Shader;
    specs.Signature = mRHI->CreateRootSignature({ RootType::PushConstant }, sizeof(glm::mat4) * 2);
    
    sData.Pipeline = mRHI->CreateGraphicsPipeline(specs);

    for (int i = 0; i < FRAMES_IN_FLIGHT; i++) {
        sData.TransferBuffer[i] = mRHI->CreateBuffer(sizeof(LineVertex) * MAX_LINES, 0, BufferType::Constant, "Line Transfer Buffer");
        sData.VertexBuffer[i] = mRHI->CreateBuffer(sizeof(LineVertex) * MAX_LINES, sizeof(LineVertex), BufferType::Vertex, "Line Vertex Buffer");
    }
}

void Debug::Render(const Frame& frame, const Scene& scene)
{
    ::Ref<RenderPassIO> cameraBuffer = PassManager::Get("CameraRingBuffer");

    if (!sData.Lines.empty()) {
        Vector<LineVertex> vertices;
        for (Line line : sData.Lines) {
            vertices.push_back({ line.From, line.Color });
            vertices.push_back({ line.To, line.Color });
        }

        sData.TransferBuffer[frame.FrameIndex]->CopyMapped(vertices.data(), vertices.size() * sizeof(LineVertex));

        glm::mat4 pushConstants[] = {
            scene.Camera.Projection(),
            scene.Camera.View()
        };

        // Copy to the vertex buffer
        frame.CommandBuffer->Barrier(sData.TransferBuffer[frame.FrameIndex], ResourceLayout::CopySource);
        frame.CommandBuffer->Barrier(sData.VertexBuffer[frame.FrameIndex], ResourceLayout::CopyDest);
        frame.CommandBuffer->CopyBufferToBuffer(sData.VertexBuffer[frame.FrameIndex], sData.TransferBuffer[frame.FrameIndex]);
        frame.CommandBuffer->Barrier(sData.VertexBuffer[frame.FrameIndex], ResourceLayout::Vertex);
        frame.CommandBuffer->Barrier(sData.TransferBuffer[frame.FrameIndex], ResourceLayout::Common);
    
        // Render
        frame.CommandBuffer->Barrier(frame.Backbuffer, ResourceLayout::ColorWrite);
        frame.CommandBuffer->SetRenderTargets({ frame.BackbufferView }, nullptr);
        frame.CommandBuffer->SetViewport(0, 0, frame.Width, frame.Height);
        frame.CommandBuffer->SetGraphicsPipeline(sData.Pipeline);
        frame.CommandBuffer->SetTopology(Topology::LineList);
        frame.CommandBuffer->SetVertexBuffer(sData.VertexBuffer[frame.FrameIndex]);
        frame.CommandBuffer->GraphicsPushConstants(pushConstants, sizeof(pushConstants), 0);
        frame.CommandBuffer->Draw(vertices.size());

        sData.Lines.clear();
    }
}

void Debug::UI()
{

}

void Debug::DrawLine(glm::vec3 from, glm::vec3 to, glm::vec3 color)
{
    sData.Lines.push_back(
        { from, to, color }
    );
}

void Debug::DrawTriangle(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 color)
{
    sData.Lines.push_back(
        { a, b, color }
    );
    sData.Lines.push_back(
        { b, c, color }
    );
    sData.Lines.push_back(
        { c, a, color }
    );
}

void Debug::DrawArrow(glm::vec3 from, glm::vec3 to, glm::vec3 color, float size)
{
    DrawLine(from, to, color);
    
    if (size > 0.0f) {
        glm::vec3 dir = to - from;
        float len = glm::length(dir);
        if (len != 0.0f)
            dir = dir * (size / len);
        else
            dir = glm::vec3(size, 0, 0);
        glm::vec3 perp = size * Math::GetNormalizedPerpendicular(dir);
        DrawLine(to - dir + perp, to, color);
        DrawLine(to - dir - perp, to, color);
    }
}
