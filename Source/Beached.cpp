//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-03 05:43:11
//

#include <Beached.hpp>

#include <Core/Logger.hpp>
#include <imgui.h>

Beached::Beached()
{
    Logger::Init();

    mWindow = MakeRef<Window>(1280, 720, "Beached <D3D12>");
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

        Frame frame = mRHI->Begin();
        
        frame.CommandBuffer->Begin();
        frame.CommandBuffer->Barrier(frame.Backbuffer, TextureLayout::ColorWrite);
        frame.CommandBuffer->ClearRenderTarget(frame.BackbufferView, 0.0f, 0.0f, 0.0f);
        frame.CommandBuffer->SetRenderTargets({ frame.BackbufferView }, nullptr);
        frame.CommandBuffer->BeginGUI(frame.Width, frame.Height);
        ImGui::ShowDemoWindow();
        frame.CommandBuffer->EndGUI();
        frame.CommandBuffer->Barrier(frame.Backbuffer, TextureLayout::Present);
        frame.CommandBuffer->End();
        
        mRHI->Submit({ frame.CommandBuffer });
        mRHI->End();
        mRHI->Present(false);
    }
}
