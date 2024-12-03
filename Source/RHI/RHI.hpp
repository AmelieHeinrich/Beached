//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-03 06:22:48
//

#pragma once

#include <Core/Window.hpp>

#include <RHI/Device.hpp>
#include <RHI/DescriptorHeap.hpp>
#include <RHI/Queue.hpp>
#include <RHI/Fence.hpp>
#include <RHI/Surface.hpp>

class RHI
{
public:
    using Ref = Ref<RHI>;

    RHI(Window::Ref window);
    ~RHI();
private:
    Device::Ref mDevice;
    Queue::Ref mGraphicsQueue;
    DescriptorHeaps mDescriptorHeaps;
    Surface::Ref mSurface;
};
