//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2025-01-18 17:35:46
//

#pragma once

#include <Renderer/RenderPass.hpp>

class AutoExposure : public RenderPass
{
public:
    AutoExposure(RHI::Ref rhi);
    ~AutoExposure() = default;

    void Bake(Scene& scene) {}
    void Render(const Frame& frame, Scene& scene) override;
    void UI(const Frame& frame) override;
};
