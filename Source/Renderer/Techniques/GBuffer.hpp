//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-21 04:31:13
//

#pragma once

#include <Renderer/RenderPass.hpp>
#include <Renderer/Permutation.hpp>

class GBuffer : public RenderPass
{
public:
    GBuffer(RHI::Ref rhi);
    ~GBuffer() = default;

    void Bake(Scene& scene) {}
    void Render(const Frame& frame, Scene& scene) override;
    void UI(const Frame& frame) override;
private:
    Sampler::Ref mSampler;
    Permutation mPipeline;
};
