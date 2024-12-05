//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-05 23:14:55
//

#pragma once

#include <RHI/RHI.hpp>

// A RenderPassIO is basically a resource that is shared between passes (a color buffer or a shadow map for instance).
struct RenderPassIO
{
    TextureDesc Desc;

    Texture::Ref Texture;
    View::Ref RenderTargetView;
    View::Ref DepthTargetView;
    View::Ref ShaderResourceView;
    View::Ref UnorderedAccessView;
};

class PassManager
{
public:
    static void Init(RHI::Ref rhi, Window::Ref window);

    static Ref<RenderPassIO> Get(const String& Name) { return sPassIOs[Name]; }
private:
    static UnorderedMap<String, Ref<RenderPassIO>> sPassIOs;
};
