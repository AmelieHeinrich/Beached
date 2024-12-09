//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-07 20:45:37
//

#pragma once

#include <Renderer/RenderPass.hpp>

constexpr int SHADOW_CASCADE_COUNT = 4;
constexpr float CASCADE_SPLIT_LAMBDA = 0.95f;

struct Cascade
{
    int SRVIndex;
    float Split;
    glm::mat4 ViewProj;
};

class CSM : public RenderPass
{
public:
    CSM(RHI::Ref rhi);
    ~CSM() = default;

    void Render(const Frame& frame, const Scene& scene) override;
    void UI(const Frame& frame) override;
private:
    void UpdateCascades(const Scene& scene);

    GraphicsPipeline::Ref mPipeline;

    bool mFreezeFrustum = false;
    glm::mat4 mFrozenView;
    glm::mat4 mFrozenProj;

    Array<float, SHADOW_CASCADE_COUNT> mSplits;
    Array<Cascade, SHADOW_CASCADE_COUNT> mCascades;
};
