//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2025-01-07 12:39:36
//

#pragma once

#include <Renderer/RenderPass.hpp>

struct PointLightShadow
{
    PointLight* Parent;
    Texture::Ref ShadowMap;

    View::Ref SRV;
    Array<View::Ref, 6> DepthViews;
};

class Shadows : public RenderPass
{
public:
    Shadows(RHI::Ref rhi);
    ~Shadows() = default;

    void Bake(const Scene& scene);
    void Render(const Frame& frame, Scene& scene) override;
    void UI(const Frame& frame) override;
private:
    GraphicsPipeline::Ref mPipeline = nullptr;
    Vector<PointLightShadow> mPointLightShadows;
};
