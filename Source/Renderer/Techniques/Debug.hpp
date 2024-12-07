//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-07 02:13:45
//

#pragma once

#include <Renderer/RenderPass.hpp>

class Debug : public RenderPass
{
public:
    struct Line
    {
        glm::vec3 From;
        glm::vec3 To;
        glm::vec3 Color;
    };

    Debug(RHI::Ref rhi);
    ~Debug() = default;

    void Render(const Frame& frame, const Scene& scene) override;
    void UI() override;

    static void DrawLine(glm::vec3 from, glm::vec3 to, glm::vec3 color = glm::vec3(1.0f));
    static void DrawTriangle(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 color = glm::vec3(1.0f));
    static void DrawArrow(glm::vec3 from, glm::vec3 to, glm::vec3 color = glm::vec3(1.0f), float size = 0.1f);
    static void DrawUnitBox(glm::mat4 transform, glm::vec3 color = glm::vec3(1.0f));
    static void DrawBox(glm::mat4 transform, glm::vec3 min, glm::vec3 max, glm::vec3 color = glm::vec3(1.0f));
    static void DrawFrustum(glm::mat4 view, glm::mat4 projection, glm::vec3 color = glm::vec3(1.0f));
    static void DrawCoordinateSystem(glm::mat4 transform, float size);
    static void DrawSphere(glm::vec3 center, float radius, glm::vec3 color = glm::vec3(1.0f), int level = 3);
private:
    static constexpr UInt32 MAX_LINES = 5192 * 8;

    static void DrawWireUnitSphereRecursive(glm::mat4 matrix, glm::vec3 inColor, glm::vec3 inDir1, glm::vec3 inDir2, glm::vec3 inDir3, int inLevel);

    struct LineVertex
    {
        glm::vec3 Position;
        glm::vec3 Color;
    };

    static struct Data
    {
        Vector<Line> Lines;
        GraphicsPipeline::Ref Pipeline;
        Array<Buffer::Ref, FRAMES_IN_FLIGHT> TransferBuffer;
        Array<Buffer::Ref, FRAMES_IN_FLIGHT> VertexBuffer;
    } sData;
};


