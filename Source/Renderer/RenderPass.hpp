//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-06 00:08:21
//

#pragma once

#include <RHI/RHI.hpp>
#include <World/Scene.hpp>
#include <Renderer/PassManager.hpp>

class RenderPass
{
public:
    using Ref = Ref<RenderPass>;

    RenderPass(RHI::Ref rhi)
        : mRHI(rhi)
    {
    }

    virtual void Render(const Frame& frame, const Scene& scene) = 0;
    virtual void UI() = 0;
protected:
    RHI::Ref mRHI;
};
