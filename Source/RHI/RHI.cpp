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
}

RHI::~RHI()
{
    
}
