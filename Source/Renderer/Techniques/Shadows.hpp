//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2025-01-07 12:39:36
//

#pragma once

#include <Renderer/RenderPass.hpp>

static const int POINT_LIGHT_SHADOW_DIMENSION = 1024;
static const int SPOT_LIGHT_SHADOW_DIMENSION = 2048;

struct PointLightShadow
{
    PointLight* Parent;
    Texture::Ref ShadowMap;

    View::Ref SRV;
    Array<View::Ref, 6> DepthViews;
};

struct SpotLightShadow
{
    SpotLight* Parent;
    Texture::Ref ShadowMap;
    
    View::Ref SRV;
    View::Ref DSV;
};

class Shadows : public RenderPass
{
public:
    Shadows(RHI::Ref rhi);
    ~Shadows() = default;

    void Bake(Scene& scene) override;
    void Render(const Frame& frame, Scene& scene) override;
    void UI(const Frame& frame) override;
private:
    Vector<SpotLightShadow> mSpotLightShadows;
    GraphicsPipeline::Ref mSpotPipeline = nullptr;

    Vector<PointLightShadow> mPointLightShadows;
    GraphicsPipeline::Ref mPointPipeline = nullptr;
};
