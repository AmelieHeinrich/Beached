//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-03 09:32:16
//

#pragma once

#include <RHI/Device.hpp>

enum class DescriptorHeapType
{
    ShaderResource = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
    Sampler = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
    RenderTarget = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
    DepthTarget = D3D12_DESCRIPTOR_HEAP_TYPE_DSV
};

class DescriptorHeap
{
public:
    using Ref = Ref<DescriptorHeap>;

    DescriptorHeap(Device::Ref device, DescriptorHeapType type, UInt32 size);
    ~DescriptorHeap();

    ID3D12DescriptorHeap* GetHeap() { return mHeap; }
private:
    DescriptorHeapType mType;
    ID3D12DescriptorHeap* mHeap;

    int mIncrementSize;
    UInt32 mHeapSize;
    bool mShaderVisible;
    Vector<bool> mLookupTable;
};

using DescriptorHeaps = UnorderedMap<DescriptorHeapType, DescriptorHeap::Ref>;
