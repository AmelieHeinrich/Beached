//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-03 06:23:27
//

#include <RHI/RHI.hpp>

RHI::RHI(Window::Ref window)
{
    mDevice = MakeRef<Device>();

    mGraphicsQueue = MakeRef<Queue>(mDevice, QueueType::AllGraphics);
    
    mDescriptorHeaps[DescriptorHeapType::RenderTarget] = MakeRef<DescriptorHeap>(mDevice, DescriptorHeapType::RenderTarget, 2048);
    mDescriptorHeaps[DescriptorHeapType::DepthTarget] = MakeRef<DescriptorHeap>(mDevice, DescriptorHeapType::DepthTarget, 2048);
    mDescriptorHeaps[DescriptorHeapType::ShaderResource] = MakeRef<DescriptorHeap>(mDevice, DescriptorHeapType::ShaderResource, 1'000'000);
    mDescriptorHeaps[DescriptorHeapType::Sampler] = MakeRef<DescriptorHeap>(mDevice, DescriptorHeapType::Sampler, 2048);

    mSurface = MakeRef<Surface>(window, mDevice, mDescriptorHeaps, mGraphicsQueue);

    mFrameFence = MakeRef<Fence>(mDevice);
    mFrameIndex = 0;
    for (int i = 0; i < FRAMES_IN_FLIGHT; i++) {
        mFrameValues[i] = 0;
        mCommandBuffers[i] = MakeRef<CommandBuffer>(mDevice, mGraphicsQueue);
    }
}

RHI::~RHI()
{
    
}

void RHI::Wait()
{
    mGraphicsQueue->Signal(mFrameFence, mFrameValues[mFrameIndex]);
    mFrameFence->Wait(mFrameValues[mFrameIndex]);
    mFrameValues[mFrameIndex]++;
}

void RHI::Submit(const Vector<CommandBuffer::Ref> buffers)
{
    mGraphicsQueue->Submit(buffers);
}

Frame RHI::Begin()
{
    Frame frame;
    frame.FrameIndex = mSurface->GetBackbufferIndex();
    frame.Backbuffer = mSurface->GetBackbuffer(frame.FrameIndex);
    frame.BackbufferView = mSurface->GetBackbufferView(frame.FrameIndex);
    frame.CommandBuffer = mCommandBuffers[frame.FrameIndex];
    
    mFrameIndex = frame.FrameIndex;

    return frame;
}

void RHI::End()
{
    const UInt64 fenceValue = mFrameValues[mFrameIndex];
    mGraphicsQueue->Signal(mFrameFence, fenceValue);

    if (mFrameFence->GetCompletedValue() < mFrameValues[mFrameIndex]) {
        mFrameFence->Wait(mFrameValues[mFrameIndex]);
    }
    mFrameValues[mFrameIndex] = fenceValue + 1;
}

void RHI::Present(bool vsync)
{
    mSurface->Present(vsync);
}
