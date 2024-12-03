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
    }
}
