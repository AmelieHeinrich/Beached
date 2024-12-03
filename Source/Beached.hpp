//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-03 05:42:43
//

#pragma once

#include <Core/Window.hpp>

#include <RHI/RHI.hpp>
#include <RHI/Uploader.hpp>

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

    Buffer::Ref mVertexBuffer;
    GraphicsPipeline::Ref mPipeline;

    // UI settings
    bool mUI = false;
};
