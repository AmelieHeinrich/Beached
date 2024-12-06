//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-03 05:43:11
//

#include <Beached.hpp>

#include <Core/Logger.hpp>
#include <UI/Helpers.hpp>
#include <Asset/AssetCacher.hpp>
#include <Renderer/PassManager.hpp>

#include <imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

Beached::Beached()
{
    Logger::Init();

    mWindow = MakeRef<Window>(1440, 900, "Beached");
    mRHI = MakeRef<RHI>(mWindow);

    AssetManager::Init(mRHI);
    AssetCacher::Init("Assets");
    PassManager::Init(mRHI, mWindow);

    mRenderer = MakeRef<Renderer>(mRHI);

    // Loading and setup
    Timer startupTimer;
    {
        mScene.Models.push_back(AssetManager::Get("Assets/Models/Sponza/Sponza.gltf", AssetType::GLTF));

        Uploader::Flush();
        mRHI->Wait();
        Uploader::ClearRequests();
    }

    LOG_INFO("Starting renderer. Startup took {0} seconds", TO_SECONDS(startupTimer.GetElapsed()));
}

Beached::~Beached()
{

}

void Beached::Run()
{
    while (mWindow->IsOpen()) {
        float time = mTimer.GetElapsed();
        float dt = time - mLastFrame;
        mLastFrame = time;
        dt /= 1000.0f;
        
        mWindow->PollEvents();

        mScene.Camera.Begin();
        if (ImGui::IsKeyPressed(ImGuiKey_F1, false)) {
            mUI = !mUI;
        }

        Frame frame = mRHI->Begin();
        frame.CommandBuffer->Begin();
        
        // Render
        {
            mRenderer->Render(frame, mScene);
        }

        // UI
        {
            frame.CommandBuffer->Barrier(frame.Backbuffer, ResourceLayout::ColorWrite);
            frame.CommandBuffer->SetRenderTargets({ frame.BackbufferView }, nullptr);
            frame.CommandBuffer->BeginGUI(frame.Width, frame.Height);
            if (mUI) {
                UI();
            } else {
                Overlay();
            }
            frame.CommandBuffer->EndGUI();
            frame.CommandBuffer->Barrier(frame.Backbuffer, ResourceLayout::Present);
        }
        
        frame.CommandBuffer->End();
        mRHI->Submit({ frame.CommandBuffer });
        mRHI->End();
        mRHI->Present(false);

        mScene.Camera.Update(dt, frame.Width, frame.Height);
    }
    mRHI->Wait();
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
