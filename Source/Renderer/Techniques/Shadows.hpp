//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2025-01-07 12:39:36
//

#pragma once

#include <Renderer/RenderPass.hpp>

constexpr int POINT_LIGHT_SHADOW_DIMENSION = 1024;
constexpr int SPOT_LIGHT_SHADOW_DIMENSION = 2048;
constexpr int SHADOW_CASCADE_COUNT = 4;

struct Cascade
{
    int SRVIndex;
    float Split;
    glm::ivec2 Pad;

    glm::mat4 View;
    glm::mat4 Proj;
};

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
    void UpdateCascades(const Scene& scene);

    float mShadowSplitLambda = 0.95f;
    bool mFreezeCascades = false;

    GraphicsPipeline::Ref mCascadePipeline = nullptr;
    Array<Cascade, SHADOW_CASCADE_COUNT> mCascades;

    Vector<SpotLightShadow> mSpotLightShadows;
    GraphicsPipeline::Ref mSpotPipeline = nullptr;

    Vector<PointLightShadow> mPointLightShadows;
    GraphicsPipeline::Ref mPointPipeline = nullptr;
};
