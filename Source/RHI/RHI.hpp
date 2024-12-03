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
#include <RHI/CommandBuffer.hpp>

struct Frame
{
    CommandBuffer::Ref CommandBuffer;
    Texture::Ref Backbuffer;
    View::Ref BackbufferView;
    UInt32 FrameIndex;

    int Width;
    int Height;
};

class RHI
{
public:
    using Ref = Ref<RHI>;

    RHI(Window::Ref window);
    ~RHI();

    void Wait();
    void Submit(const Vector<CommandBuffer::Ref> buffers);

    Frame Begin();
    void End();
    void Present(bool vsync);
private:
    Window::Ref mWindow;
    Device::Ref mDevice;
    Queue::Ref mGraphicsQueue;
    DescriptorHeaps mDescriptorHeaps;
    Surface::Ref mSurface;

    Fence::Ref mFrameFence;
    Array<UInt64, FRAMES_IN_FLIGHT> mFrameValues;
    Array<CommandBuffer::Ref, FRAMES_IN_FLIGHT> mCommandBuffers;
    UInt32 mFrameIndex;

    DescriptorHeap::Descriptor mFontDescriptor;
};
