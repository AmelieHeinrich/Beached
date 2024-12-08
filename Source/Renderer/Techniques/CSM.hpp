//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-07 20:45:37
//

#pragma once

#include <Renderer/RenderPass.hpp>

constexpr int SHADOW_CASCADE_COUNT = 4;
constexpr float SHADOW_CASCADE_LEVELS[SHADOW_CASCADE_COUNT] = { CAMERA_FAR / 50.0f, CAMERA_FAR / 25.0f, CAMERA_FAR / 10.0f, CAMERA_FAR / 2.0f };

class CSM : public RenderPass
{
public:
    CSM(RHI::Ref rhi);
    ~CSM() = default;

    void Render(const Frame& frame, const Scene& scene) override;
    void UI() override;
private:
    glm::mat4 GetLightSpaceMatrix(const Scene& scene, float near, float far);

    bool mFreezeFrustum = false;

    glm::mat4 mFrozenView;
    glm::mat4 mFrozenProj;
    Vector<glm::mat4> mLightMatrices;

    GraphicsPipeline::Ref mPipeline;
};
