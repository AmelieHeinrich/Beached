//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-06 00:24:19
//

#include <Renderer/Renderer.hpp>

#include <Renderer/Techniques/Forward.hpp>
#include <Renderer/Techniques/Composite.hpp>

Renderer::Renderer(RHI::Ref rhi)
{
    mPasses = {
        MakeRef<Forward>(rhi),
        MakeRef<Composite>(rhi)
    };
}

void Renderer::Render(const Frame& frame, const Scene& scene)
{
    for (RenderPass::Ref pass : mPasses) {
        pass->Render(frame, scene);
    }
}

void Renderer::UI()
{
    for (RenderPass::Ref pass : mPasses) {
        pass->UI();
    }
}