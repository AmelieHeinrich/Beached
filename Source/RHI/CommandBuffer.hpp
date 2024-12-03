//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-03 11:52:45
//

#pragma once

#include <RHI/Queue.hpp>
#include <RHI/View.hpp>

class CommandBuffer
{
public:
    using Ref = Ref<CommandBuffer>;
 
    CommandBuffer(Device::Ref device, Queue::Ref queue, bool singleTime = false);
    ~CommandBuffer();

    void Begin();
    void Barrier(Texture::Ref texture, TextureLayout layout, UInt32 mip = VIEW_ALL_MIPS);
    void ClearRenderTarget(View::Ref view, float r, float g, float b);
    void End();

    ID3D12GraphicsCommandList10* GetList() { return mList; }
    operator ID3D12CommandList*() { return mList; }
private:
    bool mSingleTime;
    Queue::Ref mParentQueue;
    ID3D12CommandAllocator* mAllocator;
    ID3D12GraphicsCommandList10* mList;
};
