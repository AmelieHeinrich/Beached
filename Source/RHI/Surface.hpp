//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-03 11:32:04
//

#include <RHI/Device.hpp>
#include <RHI/View.hpp>
#include <RHI/Queue.hpp>
#include <Core/Window.hpp>

constexpr UInt32 FRAMES_IN_FLIGHT = 3;

class Surface
{
public:
    using Ref = Ref<Surface>;

    Surface(Window::Ref window, Device::Ref device, DescriptorHeaps heaps, Queue::Ref queue);
    ~Surface();

private:
    IDXGISwapChain4* mSwapchain;
    Array<Texture::Ref, FRAMES_IN_FLIGHT> mBackbuffers;
    Array<View::Ref, FRAMES_IN_FLIGHT> mBackbufferViews;
};