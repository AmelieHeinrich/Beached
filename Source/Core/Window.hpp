//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-03 05:31:07
//

#pragma once

#include <Windows.h>
#include <string>

#include <Core/Common.hpp>

class Window
{
public:
    using Ref = Ref<Window>;

    Window(UInt32 width, UInt32 height, const std::string& title);
    ~Window();

    bool IsOpen();
    void PollEvents();
private:
    HWND mWindow;
};
