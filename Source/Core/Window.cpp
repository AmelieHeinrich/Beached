//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-03 05:37:35
//

#include <Core/Window.hpp>
#include <Core/Assert.hpp>

LRESULT CALLBACK WindowCallback(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    return ::DefWindowProcA(hwnd, msg, wparam, lparam);
}

Window::Window(UInt32 width, UInt32 height, const String& title)
{
    WNDCLASSA windowClass = {};
    windowClass.lpszClassName = "Beached Window Class";
    windowClass.hInstance = ::GetModuleHandleA(nullptr);
    windowClass.lpfnWndProc = WindowCallback;
    RegisterClassA(&windowClass);

    mWindow = CreateWindowA(
        windowClass.lpszClassName,
        title.data(),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        width,
        height,
        nullptr,
        nullptr,
        windowClass.hInstance,
        nullptr
    );
    ASSERT(mWindow, "Failed to create window!");

    ShowWindow(mWindow, SW_SHOW);
}

Window::~Window()
{
    DestroyWindow(mWindow);
}

bool Window::IsOpen()
{
    return IsWindowVisible(mWindow);
}

void Window::PollEvents()
{
    MSG message;
    while (PeekMessageA(&message, mWindow, 0, 0, PM_REMOVE)) {
        TranslateMessage(&message);
        DispatchMessage(&message);
    }
}
