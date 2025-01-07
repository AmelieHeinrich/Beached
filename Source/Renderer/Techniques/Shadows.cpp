//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2025-01-07 12:43:59
//

#include <Renderer/Techniques/Shadows.hpp>

Shadows::Shadows(RHI::Ref rhi)
    : RenderPass(rhi)
{

}

void Shadows::Bake(const Scene& scene)
{
    for (auto light : scene.PointLights) {
        if (light.CastShadows) {
            PointLightShadow shadow;
            shadow.Parent = &light;

            TextureDesc desc;
            desc.Name = "Point Light Shadow Map";
            desc.Usage = TextureUsage::DepthTarget;
            desc.Width = 1024;
            desc.Height = 1024;
            desc.Depth = 6;
            desc.Format = TextureFormat::Depth32;
            desc.Levels = 1;
            shadow.ShadowMap = mRHI->CreateTexture(desc);

            shadow.SRV = mRHI->CreateView(shadow.ShadowMap, ViewType::ShaderResource, ViewDimension::TextureCube, TextureFormat::R32Float);
            for (int i = 0; i < 6; i++) {
                shadow.DepthViews[i] = mRHI->CreateView(shadow.ShadowMap, ViewType::DepthTarget, ViewDimension::TextureCube, TextureFormat::Depth32, VIEW_ALL_MIPS, i);
            }

            mPointLightShadows.push_back(shadow);
        }
    }
}

void Shadows::Render(const Frame& frame, Scene& scene)
{

}

void Shadows::UI(const Frame& frame)
{

}
