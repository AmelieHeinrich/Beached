//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-03 05:42:43
//

#pragma once

#include <Core/Window.hpp>
#include <Core/Timer.hpp>

#include <Asset/AssetManager.hpp>

#include <World/Camera.hpp>
#include <World/Scene.hpp>

#include <RHI/RHI.hpp>
#include <RHI/Uploader.hpp>
#include <RHI/Sampler.hpp>

#include <Renderer/Renderer.hpp>

class Beached
{
public:
    Beached();
    ~Beached();

    void Run();
private:
    void Overlay();
    void UI();

    Window::Ref mWindow;
    RHI::Ref mRHI;
    Renderer::Ref mRenderer;

    Timer mTimer;
    float mLastFrame;
    Scene mScene;

    // UI settings
    bool mUI = false;
    bool mRendererUI = false;
};
