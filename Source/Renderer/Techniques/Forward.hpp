//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-06 00:12:29
//

#pragma once

#include <Renderer/RenderPass.hpp>

class Forward : public RenderPass
{
public:
    Forward(RHI::Ref rhi);
    ~Forward() = default;

    void Render(const Frame& frame, const Scene& scene) override;
    void UI() override;
private:
    Sampler::Ref mSampler;
    GraphicsPipeline::Ref mPipeline;
};
