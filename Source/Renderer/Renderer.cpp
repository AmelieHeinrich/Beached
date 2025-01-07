//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-06 00:24:19
//

#include <Renderer/Renderer.hpp>

#include <Renderer/Techniques/CSM.hpp>
#include <Renderer/Techniques/Shadows.hpp>
#include <Renderer/Techniques/GBuffer.hpp>
#include <Renderer/Techniques/Forward.hpp>
#include <Renderer/Techniques/Composite.hpp>
#include <Renderer/Techniques/Debug.hpp>

#include <Settings.hpp>
#include <imgui.h>

Renderer::Renderer(RHI::Ref rhi)
{
    mPasses = {
        MakeRef<CSM>(rhi),
        MakeRef<Shadows>(rhi),
        MakeRef<GBuffer>(rhi),
        MakeRef<Forward>(rhi),
        MakeRef<Composite>(rhi),
        MakeRef<Debug>(rhi)
    };
}

Renderer::~Renderer()
{
    mPasses.clear();
}

void Renderer::Bake(const Scene& scene)
{
    for (auto& pass : mPasses) {
        pass->Bake(scene);
    }
}

void Renderer::Render(const Frame& frame, Scene& scene)
{
    for (auto& pass : mPasses) {
        pass->Render(frame, scene);
    }
}

void Renderer::UI(const Frame& frame, bool *open)
{
    if (*open) {
        ImGui::Begin("Renderer", open);
        if (ImGui::TreeNodeEx("Global Settings", ImGuiTreeNodeFlags_Framed)) {
            ImGui::Checkbox("Scene Use Sun", &Settings::Get().SceneUseSun);
            ImGui::Checkbox("Draw Scene OBB", &Settings::Get().DebugDrawSceneOOB);
            ImGui::Checkbox("Frustum Cull", &Settings::Get().FrustumCull);
            ImGui::Checkbox("Freeze Frustum", &Settings::Get().FreezeFrustum);
            ImGui::TreePop();
        }
        for (auto& pass : mPasses) {
            pass->UI(frame);
        }
        ImGui::End();
    }
}
