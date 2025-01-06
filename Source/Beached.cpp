//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-03 05:43:11
//

#include <Beached.hpp>

#include <Core/Random.hpp>
#include <Core/Logger.hpp>
#include <UI/Helpers.hpp>
#include <Asset/AssetCacher.hpp>
#include <Renderer/PassManager.hpp>
#include <Renderer/Techniques/Debug.hpp>

#include <Statistics.hpp>
#include <imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <sstream>

Beached::Beached()
{
    Timer startupTimer;
    {
        Logger::Init();

        mWindow = MakeRef<Window>(1920, 1080, "Beached");
        mRHI = MakeRef<RHI>(mWindow);

        AssetManager::Init(mRHI);
        AssetCacher::Init("Assets");
        PassManager::Init(mRHI, mWindow);

        mRenderer = MakeRef<Renderer>(mRHI);

        // Loading and setup
        mScene.Models.push_back(AssetManager::Get("Assets/Models/ShadowTest/ShadowTest.gltf", AssetType::GLTF));
        mScene.Sun.Direction = glm::vec3(0.0f, -1.0f, 0.2f);
        mScene.Sun.Color = glm::vec4(1.0f);
        mScene.Sun.Strenght = 1.0f;

        // Add lights
        // for (int i = 0; i < 128; i++) {
        //     PointLight light;
        //     light.Position = glm::vec3(Random::Float(-5.0f, 5.0f), Random::Float(0.0f, 5.0f), Random::Float(-5.0f, 5.0f));
        //     light.Color = glm::vec4(Random::Float(0.0f, 1.0f), Random::Float(0.0f, 1.0f), Random::Float(0.0f, 1.0f), 1.0f);
        //     light.Radius = Random::Float(0.05f, 0.3f);
        //     mScene.PointLights.push_back(light);
        // }

        mScene.Init(mRHI);
        Uploader::Flush();
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

        mScene.Camera.FreezeFrustum(Settings::Get().FreezeFrustum);
        if (!Settings::Get().FreezeFrustum) {
            mFrozenView = mScene.Camera.View();
            mFrozenProj = mScene.Camera.Projection();
        } else {
            Debug::DrawFrustum(mFrozenProj * mFrozenView, glm::vec3(1.0f, 0.0f, 0.0f));
        }

        mScene.Camera.Begin();
        if (ImGui::IsKeyPressed(ImGuiKey_F1, false)) {
            mUI = !mUI;
        }

        Frame frame = mRHI->Begin();
        frame.CommandBuffer->Begin();

        // Render
        {
            mScene.Update(frame, frame.FrameIndex);
            mRenderer->Render(frame, mScene);
        }

        // UI
        {
            frame.CommandBuffer->BeginMarker("ImGui");
            frame.CommandBuffer->Barrier(frame.Backbuffer, ResourceLayout::ColorWrite);
            frame.CommandBuffer->SetRenderTargets({ frame.BackbufferView }, nullptr);
            frame.CommandBuffer->BeginGUI(frame.Width, frame.Height);
            if (mUI) {
                UI(frame);
            } else {
                Overlay();
            }
            frame.CommandBuffer->EndGUI();
            frame.CommandBuffer->Barrier(frame.Backbuffer, ResourceLayout::Present);
            frame.CommandBuffer->EndMarker();
        }
        
        frame.CommandBuffer->End();
        mRHI->Submit({ frame.CommandBuffer });
        mRHI->End();
        mRHI->Present(false);

        ImGuiIO& io = ImGui::GetIO();
        if (!io.WantCaptureMouse)
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

void Beached::UI(const Frame& frame)
{
    Settings& settings = Settings::Get();

    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("Window")) {
            if (ImGui::MenuItem("Quit")) {
                // todo
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Settings")) {
            if (ImGui::MenuItem("Render Settings")) {
                mRendererUI = !mRendererUI;
            }
            if (ImGui::MenuItem("Statistics")) {
                mStatisticsUI = !mStatisticsUI;
            }
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    UI::BeginCornerOverlay();
    ImGui::Text("Version 0.0.1");
    ImGui::Text("Renderer: Direct3D 12");
    ImGui::End();

    mRenderer->UI(frame, &mRendererUI);

    if (mStatisticsUI) {
        Statistics::Update();
        ImGui::Begin("Statistics", &mStatisticsUI);

        // TODO: Frame times

        // Geometry and RHI
        ImGui::Text("Instance Count : %llu", Statistics::Get().InstanceCount);
        ImGui::Text("Culled Instances : %llu", Statistics::Get().CulledInstances);
        ImGui::Text("Triangle Count : %llu", Statistics::Get().TriangleCount);
        ImGui::Text("Culled Triangles : %llu", Statistics::Get().CulledTriangles);
        ImGui::Text("Draw Call Count : %llu", Statistics::Get().DrawCallCount);
        ImGui::Text("Dispatch Count : %llu", Statistics::Get().DispatchCount);

        //
        ImGui::Separator();
        //

        // Resources
        // VRAM
        {
            UInt64 percentage = (Statistics::Get().UsedVRAM * 100) / Statistics::Get().MaxVRAM;
            float stupidVRAMPercetange = percentage / 100.0f;

            std::stringstream ss;
            ss << "VRAM Usage (" << percentage << "%%): " << (((Statistics::Get().UsedVRAM / 1024.0F) / 1024.0f) / 1024.0f) << "gb/" << (((Statistics::Get().MaxVRAM / 1024.0F) / 1024.0f) / 1024.0f) << "gb";

            std::stringstream percents;
            percents << percentage << "%";

            ImGui::Text(ss.str().c_str());
            ImGui::ProgressBar(stupidVRAMPercetange, ImVec2(0, 0), percents.str().c_str());
        }

        // RAM
        {
            UInt64 percentage = (Statistics::Get().UsedRAM * 100) / Statistics::Get().MaxRAM;
            float stupidRAMPercetange = percentage / 100.0f;

            std::stringstream ss;
            ss << "RAM Usage (" << percentage << "%%): " << (((Statistics::Get().UsedRAM / 1024.0F) / 1024.0f) / 1024.0f) << "gb/" << (((Statistics::Get().MaxRAM / 1024.0F) / 1024.0f) / 1024.0f) << "gb";

            std::stringstream percents;
            percents << percentage << "%";

            ImGui::Text(ss.str().c_str());
            ImGui::ProgressBar(stupidRAMPercetange, ImVec2(0, 0), percents.str().c_str());
        }

        // Battery
        {
            std::stringstream ss;
            ss << "Battery (" << Statistics::Get().Battery << "%%)";

            std::stringstream percentss;
            percentss << Statistics::Get().Battery << "%";

            ImGui::Text(ss.str().c_str());
            ImGui::ProgressBar(Statistics::Get().Battery / 100.0f, ImVec2(0, 0), percentss.str().c_str());
        }

        ImGui::End();
    }
}
