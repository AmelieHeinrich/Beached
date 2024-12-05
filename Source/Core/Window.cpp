//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-03 05:37:35
//

#include <Core/Window.hpp>
#include <Core/Assert.hpp>

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK WindowCallback(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam))
        return 1;

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
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
        (1920 - width) / 2,
        (1080 - height) / 2,
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

void Window::PollSize(int& width, int& height)
{
    RECT rect;
    GetClientRect(mWindow, &rect);
    width = rect.right - rect.left;
    height = rect.bottom - rect.top;
}
