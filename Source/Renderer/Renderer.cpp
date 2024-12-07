//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-06 00:24:19
//

#include <Renderer/Renderer.hpp>

#include <Renderer/Techniques/CSM.hpp>
#include <Renderer/Techniques/Forward.hpp>
#include <Renderer/Techniques/Composite.hpp>
#include <Renderer/Techniques/Debug.hpp>

#include <imgui.h>

Renderer::Renderer(RHI::Ref rhi)
{
    mPasses = {
        MakeRef<CSM>(rhi),
        MakeRef<Forward>(rhi),
        MakeRef<Composite>(rhi),
        MakeRef<Debug>(rhi)
    };
}

void Renderer::Render(const Frame& frame, const Scene& scene)
{
    for (RenderPass::Ref pass : mPasses) {
        pass->Render(frame, scene);
    }
}

void Renderer::UI(bool *open)
{
    if (*open) {
        ImGui::Begin("Renderer", open);
        for (RenderPass::Ref pass : mPasses) {
            pass->UI();
        }
        ImGui::End();
    }
}
