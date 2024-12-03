//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-03 05:43:11
//

#include <Beached.hpp>

#include <Core/Logger.hpp>
#include <UI/Helpers.hpp>

#include <imgui.h>

Beached::Beached()
{
    Logger::Init();

    mWindow = MakeRef<Window>(1440, 900, "Beached");
    mRHI = MakeRef<RHI>(mWindow);

    mRHI->Wait();
    LOG_INFO("Starting Beached");
}

Beached::~Beached()
{

}

void Beached::Run()
{
    while (mWindow->IsOpen()) {
        mWindow->PollEvents();

        if (ImGui::IsKeyPressed(ImGuiKey_F1, false)) {
            mUI = !mUI;
        }

        Frame frame = mRHI->Begin();
        
        frame.CommandBuffer->Begin();
        frame.CommandBuffer->Barrier(frame.Backbuffer, TextureLayout::ColorWrite);
        frame.CommandBuffer->ClearRenderTarget(frame.BackbufferView, 0.0f, 0.0f, 0.0f);
        frame.CommandBuffer->SetRenderTargets({ frame.BackbufferView }, nullptr);
        frame.CommandBuffer->BeginGUI(frame.Width, frame.Height);
        if (mUI) {
            UI();
        } else {
            Overlay();
        }
        frame.CommandBuffer->EndGUI();
        frame.CommandBuffer->Barrier(frame.Backbuffer, TextureLayout::Present);
        frame.CommandBuffer->End();
        
        mRHI->Submit({ frame.CommandBuffer });
        mRHI->End();
        mRHI->Present(false);
    }
}

void Beached::Overlay()
{
    UI::BeginCornerOverlay();
    ImGui::Text("Debug Menu: F1");
    ImGui::End();
}

void Beached::UI()
{
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("Window")) {
            if (ImGui::MenuItem("Quit")) {
                // todo
            }
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    UI::BeginCornerOverlay();
    ImGui::Text("Version 0.0.1");
    ImGui::Text("Renderer: Direct3D 12");
    ImGui::End();
}
