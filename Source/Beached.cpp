//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-03 05:43:11
//

#include <Core/Logger.hpp>

#include <Beached.hpp>

Beached::Beached()
{
    Logger::Init();

    mWindow = MakeRef<Window>(1280, 720, "Beached <D3D12>");
    mRHI = MakeRef<RHI>(mWindow);

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
        frame.CommandBuffer->ClearRenderTarget(frame.BackbufferView, 0.3f, 0.8f, 0.1f);
        frame.CommandBuffer->Barrier(frame.Backbuffer, TextureLayout::Present);
        frame.CommandBuffer->End();
        mRHI->Submit({ frame.CommandBuffer });
        mRHI->End();
        mRHI->Present(false);
    }
}
