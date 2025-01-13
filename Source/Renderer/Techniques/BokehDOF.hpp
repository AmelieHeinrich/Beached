//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2025-01-11 06:44:00
//

#pragma once

#include <Renderer/RenderPass.hpp>

class BokehDOF : public RenderPass
{
public:
    BokehDOF(RHI::Ref rhi);
    ~BokehDOF() = default;

    void Bake(Scene& scene) {}
    void Render(const Frame& frame, Scene& scene) override;
    void UI(const Frame& frame) override;
private:
    ComputePipeline::Ref mCOCGeneration;
    Sampler::Ref mPointSampler;

    float mFocalPoint = 20.0f;
};
