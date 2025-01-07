//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-06 00:21:24
//

#pragma once

#include <Renderer/RenderPass.hpp>

class Composite : public RenderPass
{
public:
    Composite(RHI::Ref rhi);
    ~Composite() = default;

    void Render(const Frame& frame, Scene& scene) override;
    void UI(const Frame& frame) override;
private:
    ComputePipeline::Ref mPipeline;
    RootSignature::Ref mSignature;
};
