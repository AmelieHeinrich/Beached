//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-03 05:42:43
//

#pragma once

#include <Core/Window.hpp>
#include <RHI/RHI.hpp>

class Beached
{
public:
    Beached();
    ~Beached();

    void Run();
private:
    Window::Ref mWindow;
    RHI::Ref mRHI;
};
