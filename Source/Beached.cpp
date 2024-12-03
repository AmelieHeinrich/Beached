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

    GraphicsPipelineSpecs triangleSpecs;
    triangleSpecs.Fill = FillMode::Solid;
    triangleSpecs.Cull = CullMode::None;
    triangleSpecs.Formats.push_back(TextureFormat::RGBA8);
    triangleSpecs.DepthEnabled = false;
    triangleSpecs.Bytecodes[ShaderType::Vertex] = ShaderCompiler::Compile("Assets/Shaders/StreamedTriangle/Vertex.hlsl", "VSMain", ShaderType::Vertex);
    triangleSpecs.Bytecodes[ShaderType::Fragment] = ShaderCompiler::Compile("Assets/Shaders/StreamedTriangle/Fragment.hlsl", "PSMain", ShaderType::Fragment);
    triangleSpecs.Signature = mRHI->CreateRootSignature();

    mPipeline = mRHI->CreateGraphicsPipeline(triangleSpecs);

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

        int width, height;
        mWindow->PollSize(width, height);

        if (ImGui::IsKeyPressed(ImGuiKey_F1, false)) {
            mUI = !mUI;
        }

        Frame frame = mRHI->Begin();
        
        frame.CommandBuffer->Begin();
        frame.CommandBuffer->Barrier(frame.Backbuffer, ResourceLayout::ColorWrite);
        frame.CommandBuffer->ClearRenderTarget(frame.BackbufferView, 0.0f, 0.0f, 0.0f);
        frame.CommandBuffer->SetRenderTargets({ frame.BackbufferView }, nullptr);
       
        // Triangle
        frame.CommandBuffer->SetTopology(Topology::TriangleList);
        frame.CommandBuffer->SetViewport(0, 0, (float)width, (float)height);
        frame.CommandBuffer->SetGraphicsPipeline(mPipeline);
        frame.CommandBuffer->Draw(3);

        // UI
        frame.CommandBuffer->BeginGUI(width, height);
        if (mUI) {
            UI();
        } else {
            Overlay();
        }
        frame.CommandBuffer->EndGUI();
        
        frame.CommandBuffer->Barrier(frame.Backbuffer, ResourceLayout::Present);
        frame.CommandBuffer->End();
        
        mRHI->Submit({ frame.CommandBuffer });
        mRHI->End();
        mRHI->Present(false);
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
