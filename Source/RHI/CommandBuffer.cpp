//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-03 11:56:02
//

#include <RHI/CommandBuffer.hpp>
#include <RHI/Utilities.hpp>
#include <Core/Assert.hpp>

CommandBuffer::CommandBuffer(Device::Ref device, Queue::Ref queue, bool singleTime)
    : mSingleTime(singleTime), mParentQueue(queue)
{
    HRESULT result = device->GetDevice()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE(queue->GetType()), IID_PPV_ARGS(&mAllocator));
    ASSERT(SUCCEEDED(result), "Failed to create command allocator!");

    result = device->GetDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE(queue->GetType()), mAllocator, nullptr, IID_PPV_ARGS(&mList));
    ASSERT(SUCCEEDED(result), "Failed to create command list!");

    if (!singleTime) {
        mList->Close();
    }
}

CommandBuffer::~CommandBuffer()
{
    D3DUtils::Release(mAllocator);
    D3DUtils::Release(mList);
}

void CommandBuffer::Begin()
{
    if (!mSingleTime) {
        mAllocator->Reset();
        mList->Reset(mAllocator, nullptr);
    }

    // TODO(amelie): set descriptor heaps
}

void CommandBuffer::Barrier(Texture::Ref texture, TextureLayout layout, UInt32 mip)
{
    D3D12_RESOURCE_BARRIER Barrier = {};
    Barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    Barrier.Transition.pResource = texture->GetResource();
    Barrier.Transition.StateBefore = D3D12_RESOURCE_STATES(texture->GetLayout());
    Barrier.Transition.StateAfter = D3D12_RESOURCE_STATES(layout);
    Barrier.Transition.Subresource = mip == VIEW_ALL_MIPS ? D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES : mip;
    
    if (Barrier.Transition.StateBefore == D3D12_RESOURCE_STATE_UNORDERED_ACCESS && Barrier.Transition.StateAfter == D3D12_RESOURCE_STATE_UNORDERED_ACCESS) {
        Barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
        Barrier.UAV.pResource = texture->GetResource();
    } else {
        if (Barrier.Transition.StateBefore == Barrier.Transition.StateAfter)
            return;
    }
    
    mList->ResourceBarrier(1, &Barrier);
    texture->SetLayout(layout);
}

void CommandBuffer::ClearRenderTarget(View::Ref view, float r, float g, float b)
{
    float clear[] = { r, g, b, 1.0f };
    mList->ClearRenderTargetView(view->GetDescriptor().CPU, clear, 0, nullptr);
}

void CommandBuffer::End()
{
    mList->Close();
}
