//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-03 15:32:45
//

#include <UI/Helpers.hpp>

#include <imgui.h>

void UI::BeginCornerOverlay()
{
    static bool open = true;

    ImGuiIO& io = ImGui::GetIO();
    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoMove;
    const float pad = 10.0f;
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 workPos = viewport->WorkPos; // Use work area to avoid menu-bar/task-bar, if any!
    ImVec2 workSize = viewport->WorkSize;
    ImVec2 windowPos = ImVec2(workPos.x + pad, workPos.y + pad);

    ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always, ImVec2(0, 0));
    ImGui::SetNextWindowBgAlpha(0.70f);
    ImGui::Begin("Overlay", &open, windowFlags);
}
