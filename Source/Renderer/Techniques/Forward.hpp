//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-06 00:12:29
//

#pragma once

#include <Renderer/RenderPass.hpp>
#include <Renderer/Permutation.hpp>

class Forward : public RenderPass
{
public:
    Forward(RHI::Ref rhi);
    ~Forward() = default;

    void Bake(Scene& scene) {}
    void Render(const Frame& frame, Scene& scene) override;
    void UI(const Frame& frame) override;
private:
    Sampler::Ref mSampler;
    Sampler::Ref mClampSampler;
    Sampler::Ref mShadowSampler;
    Permutation mPipeline;

    int mCulledOBBs = 0;
};
