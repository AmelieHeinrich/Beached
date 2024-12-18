//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-03 06:23:27
//

#include <RHI/RHI.hpp>
#include <RHI/Uploader.hpp>

#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx12.h>

RHI::RHI(Window::Ref window)
    : mWindow(window)
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
        mCommandBuffers[i] = MakeRef<CommandBuffer>(mDevice, mGraphicsQueue, mDescriptorHeaps);
    }

    Uploader::Init(mDevice, mDescriptorHeaps, mGraphicsQueue);

    mFontDescriptor = mDescriptorHeaps[DescriptorHeapType::ShaderResource]->Allocate();

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsLight();

    ImGuiIO& IO = ImGui::GetIO();
    IO.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    IO.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    IO.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui_ImplWin32_EnableDpiAwareness();
    ImGui_ImplWin32_Init(window->GetHandle());
    ImGui_ImplDX12_Init(mDevice->GetDevice(),
                        FRAMES_IN_FLIGHT,
                        DXGI_FORMAT_R8G8B8A8_UNORM,
                        mDescriptorHeaps[DescriptorHeapType::RenderTarget]->GetHeap(),
                        mFontDescriptor.CPU,
                        mFontDescriptor.GPU);
}

RHI::~RHI()
{
    Wait();

    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    mFontDescriptor.Parent->Free(mFontDescriptor);
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
    Frame frame = {};
    frame.FrameIndex = mSurface->GetBackbufferIndex();
    frame.Backbuffer = mSurface->GetBackbuffer(frame.FrameIndex);
    frame.BackbufferView = mSurface->GetBackbufferView(frame.FrameIndex);
    frame.CommandBuffer = mCommandBuffers[frame.FrameIndex];
    
    mFrameIndex = frame.FrameIndex;

    mWindow->PollSize(frame.Width, frame.Height);

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

RootSignature::Ref RHI::CreateRootSignature()
{
    return MakeRef<RootSignature>(mDevice);
}

RootSignature::Ref RHI::CreateRootSignature(const Vector<RootType>& entries, UInt64 pushConstantSize)
{
    return MakeRef<RootSignature>(mDevice, entries, pushConstantSize);
}

GraphicsPipeline::Ref RHI::CreateGraphicsPipeline(GraphicsPipelineSpecs& specs)
{
    return MakeRef<GraphicsPipeline>(mDevice, specs);
}

ComputePipeline::Ref RHI::CreateComputePipeline(Shader shader, RootSignature::Ref signature)
{
    return MakeRef<ComputePipeline>(mDevice, shader, signature);
}

Buffer::Ref RHI::CreateBuffer(UInt64 size, UInt64 stride, BufferType type, const String& name)
{
    return MakeRef<Buffer>(mDevice, mDescriptorHeaps, size, stride, type, name);
}

Texture::Ref RHI::CreateTexture(TextureDesc desc)
{
    return MakeRef<Texture>(mDevice, desc);
}

View::Ref RHI::CreateView(::Ref<Resource> resource, ViewType type, ViewDimension dimension, TextureFormat format, UInt64 mip, UInt64 depthSlice)
{
    return MakeRef<View>(mDevice, mDescriptorHeaps, resource, type, dimension, format, mip, depthSlice);
}

Sampler::Ref RHI::CreateSampler(SamplerAddress address, SamplerFilter filter, bool mips, int anisotropyLevel)
{
    return MakeRef<Sampler>(mDevice, mDescriptorHeaps, address, filter, mips, anisotropyLevel);
}

BLAS::Ref RHI::CreateBLAS(Buffer::Ref vertex, Buffer::Ref index, UInt32 vtxCount, UInt32 idxCount, const String& name)
{
    return MakeRef<BLAS>(mDevice, mDescriptorHeaps, vertex, index, vtxCount, idxCount, name);
}

TLAS::Ref RHI::CreateTLAS(Buffer::Ref instanceBuffer, UInt32 numInstance, const String& name)
{
    return MakeRef<TLAS>(mDevice, mDescriptorHeaps, instanceBuffer, numInstance, name);
}
