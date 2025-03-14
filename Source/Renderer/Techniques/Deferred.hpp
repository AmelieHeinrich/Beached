//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-03-14 02:10:30
//

#pragma once

#include <Renderer/RenderPass.hpp>
#include <Renderer/Permutation.hpp>

class Deferred : public RenderPass
{
public:
    Deferred(RHI::Ref rhi);
    ~Deferred() = default;

    void Bake(Scene& scene) {}
    void Render(const Frame& frame, Scene& scene) override;
    void UI(const Frame& frame) override;
private:
    Sampler::Ref mSampler;
    Sampler::Ref mClampSampler;
    Sampler::Ref mShadowSampler;
    ComputePipeline::Ref mPipeline;
};
