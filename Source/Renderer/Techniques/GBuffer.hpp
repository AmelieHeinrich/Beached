//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-21 04:31:13
//

#pragma once

#include <Renderer/RenderPass.hpp>

class GBuffer : public RenderPass
{
public:
    GBuffer(RHI::Ref rhi);
    ~GBuffer() = default;

    void Render(const Frame& frame, const Scene& scene) override;
    void UI(const Frame& frame) override;
private:
    Sampler::Ref mSampler;
    GraphicsPipeline::Ref mPipeline;
};
