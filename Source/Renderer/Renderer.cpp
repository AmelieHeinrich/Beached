//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-06 00:24:19
//

#include <Renderer/Renderer.hpp>

#include <Renderer/Techniques/GBuffer.hpp>
#include <Renderer/Techniques/CSM.hpp>
#include <Renderer/Techniques/Forward.hpp>
#include <Renderer/Techniques/Composite.hpp>
#include <Renderer/Techniques/Debug.hpp>

#include <Settings.hpp>
#include <imgui.h>

Renderer::Renderer(RHI::Ref rhi)
{
    mPasses = {
        MakeRef<CSM>(rhi),
        MakeRef<GBuffer>(rhi),
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

void Renderer::UI(const Frame& frame, bool *open)
{
    if (*open) {
        ImGui::Begin("Renderer", open);
        if (ImGui::TreeNodeEx("Global Settings", ImGuiTreeNodeFlags_Framed)) {
            ImGui::Checkbox("Frustum Cull", &Settings::Get().FrustumCull);
            ImGui::Checkbox("Freeze Frustum", &Settings::Get().FreezeFrustum);
            ImGui::TreePop();
        }
        for (RenderPass::Ref pass : mPasses) {
            pass->UI(frame);
        }
        ImGui::End();
    }
}
