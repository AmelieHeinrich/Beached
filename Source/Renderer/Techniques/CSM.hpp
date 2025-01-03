//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-07 20:45:37
//

#pragma once

#include <Renderer/RenderPass.hpp>

constexpr int SHADOW_CASCADE_COUNT = 4;
constexpr float SHADOW_SPLIT_LAMBDA = 0.5f;

struct Cascade
{
    int SRVIndex;
    float Split;
    glm::mat4 View;
    glm::mat4 Proj;
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

    bool mFreezeCascades = false;
    Array<Cascade, SHADOW_CASCADE_COUNT + 1> mCascades;
};
