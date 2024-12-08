//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-07 20:45:37
//

#pragma once

#include <Renderer/RenderPass.hpp>

constexpr int SHADOW_CASCADE_COUNT = 4;

class CSM : public RenderPass
{
public:
    struct LightMatrix {
        bool Visualized = false;
        glm::mat4 View;
        glm::mat4 Projection;
    };

    CSM(RHI::Ref rhi);
    ~CSM() = default;

    void Render(const Frame& frame, const Scene& scene) override;
    void UI(const Frame& frame) override;

    Array<float, SHADOW_CASCADE_COUNT + 1> ComputeCascadeLevels(float cameraNear, float cameraFar);
private:
    LightMatrix GetLightSpaceMatrix(const Frame& frame, const Scene& scene, float near, float far);

    bool mFreezeFrustum = false;

    glm::mat4 mFrozenView;
    glm::mat4 mFrozenProj;
    Vector<LightMatrix> mLightMatrices;

    GraphicsPipeline::Ref mPipeline;
};
