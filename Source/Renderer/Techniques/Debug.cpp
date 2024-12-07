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

void Debug::DrawUnitBox(glm::mat4 transform, glm::vec3 color)
{
    DrawBox(transform, glm::vec3(-1.0f), glm::vec3(1.0f), color);
}

void Debug::DrawBox(glm::mat4 transform, glm::vec3 min, glm::vec3 max, glm::vec3 color)
{
    glm::vec3 v1 = transform * glm::vec4(min.x, min.y, min.z, 1.0);
	glm::vec3 v2 = transform * glm::vec4(min.x, min.y, max.z, 1.0);
	glm::vec3 v3 = transform * glm::vec4(min.x, max.y, min.z, 1.0);
	glm::vec3 v4 = transform * glm::vec4(min.x, max.y, max.z, 1.0);
	glm::vec3 v5 = transform * glm::vec4(max.x, min.y, min.z, 1.0);
	glm::vec3 v6 = transform * glm::vec4(max.x, min.y, max.z, 1.0);
	glm::vec3 v7 = transform * glm::vec4(max.x, max.y, min.z, 1.0);
	glm::vec3 v8 = transform * glm::vec4(max.x, max.y, max.z, 1.0);

	// 12 edges
	DrawLine(v1, v2, color);
	DrawLine(v1, v3, color);
	DrawLine(v1, v5, color);
	DrawLine(v2, v4, color);
	DrawLine(v2, v6, color);
	DrawLine(v3, v4, color);
	DrawLine(v3, v7, color);
	DrawLine(v4, v8, color);
	DrawLine(v5, v6, color);
	DrawLine(v5, v7, color);
	DrawLine(v6, v8, color);
	DrawLine(v7, v8, color);
}

void Debug::DrawFrustum(glm::mat4 view, glm::mat4 projection, glm::vec3 color)
{
    glm::vec3 corners[8] = {
        glm::vec3(-1.0f,  1.0f, 0.0f),
        glm::vec3( 1.0f,  1.0f, 0.0f),
        glm::vec3( 1.0f, -1.0f, 0.0f),
        glm::vec3(-1.0f, -1.0f, 0.0f),
        glm::vec3(-1.0f,  1.0f, 1.0f),
        glm::vec3( 1.0f,  1.0f, 1.0f),
        glm::vec3( 1.0f, -1.0f, 1.0f),
        glm::vec3(-1.0f, -1.0f, 1.0f),
    };

    // To convert from world space to NDC space, multiply by the inverse of the camera matrix (projection * view) then perspective divide
    // Not sure I 100% understand the math here, TODO: study
    for (int i = 0; i < 8; i++) {
        glm::vec4 v = glm::vec4(corners[i], 1.0);
        glm::vec4 h = glm::inverse(projection * view) * v;
        h.x /= h.w;
        h.y /= h.w;
        h.z /= h.w;
        corners[i] = glm::vec3(h);
    }

    for (int i = 0; i < 4; i++) {
        DrawLine(corners[i % 4],     corners[(i + 1) % 4],     color);
        DrawLine(corners[i],         corners[i + 4],           color);
        DrawLine(corners[i % 4 + 4], corners[(i + 1) % 4 + 4], color);
    }
}

void Debug::DrawCoordinateSystem(glm::mat4 transform, float size)
{
    glm::vec3 translation = glm::vec3(transform[0][3], transform[1][3], transform[2][3]);
    DrawArrow(translation, transform * glm::vec4(size, 0, 0, 1.0f), glm::vec3(1.0f, 0.0f, 0.0f), 0.1f * size);
	DrawArrow(translation, transform * glm::vec4(0, size, 0, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), 0.1f * size);
	DrawArrow(translation, transform * glm::vec4(0, 0, size, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f), 0.1f * size);
}

void Debug::DrawSphere(glm::vec3 center, float radius, glm::vec3 color, int level)
{
    glm::mat4 matrix = glm::translate(glm::mat4(1.0f), center) * glm::scale(glm::mat4(1.0f), glm::vec3(radius));
    glm::vec3 xAxis = glm::vec3(1.0f, 0.0f, 0.0f);
    glm::vec3 yAxis = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 zAxis = glm::vec3(0.0f, 0.0f, 1.0f);

    DrawWireUnitSphereRecursive(matrix, color,  xAxis,  yAxis,  zAxis, level);
	DrawWireUnitSphereRecursive(matrix, color, -xAxis,  yAxis,  zAxis, level);
	DrawWireUnitSphereRecursive(matrix, color,  xAxis, -yAxis,  zAxis, level);
	DrawWireUnitSphereRecursive(matrix, color, -xAxis, -yAxis,  zAxis, level);
	DrawWireUnitSphereRecursive(matrix, color,  xAxis,  yAxis, -zAxis, level);
	DrawWireUnitSphereRecursive(matrix, color, -xAxis,  yAxis, -zAxis, level);
	DrawWireUnitSphereRecursive(matrix, color,  xAxis, -yAxis, -zAxis, level);
	DrawWireUnitSphereRecursive(matrix, color, -xAxis, -yAxis, -zAxis, level);
}

void Debug::DrawWireUnitSphereRecursive(glm::mat4 matrix, glm::vec3 inColor, glm::vec3 inDir1, glm::vec3 inDir2, glm::vec3 inDir3, int inLevel)
{
    if (inLevel == 0) {
		glm::vec3 d1 = matrix * glm::vec4(inDir1, 1.0f);
		glm::vec3 d2 = matrix * glm::vec4(inDir2, 1.0f);
		glm::vec3 d3 = matrix * glm::vec4(inDir3, 1.0f);

		DrawLine(d1, d2, inColor);
		DrawLine(d2, d3, inColor);
		DrawLine(d3, d1, inColor);
	} else {
		glm::vec3 center1 = glm::normalize(inDir1 + inDir2);
		glm::vec3 center2 = glm::normalize(inDir2 + inDir3);
		glm::vec3 center3 = glm::normalize(inDir3 + inDir1);

		DrawWireUnitSphereRecursive(matrix, inColor, inDir1, center1, center3, inLevel - 1);
		DrawWireUnitSphereRecursive(matrix, inColor, center1, center2, center3, inLevel - 1);
		DrawWireUnitSphereRecursive(matrix, inColor, center1, inDir2, center2, inLevel - 1);
		DrawWireUnitSphereRecursive(matrix, inColor, center3, center2, inDir3, inLevel - 1);
	}
}
