//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-03 10:38:04
//

#pragma once

#include <RHI/Device.hpp>

// We go through a parent Resource class so it's easier to tag/track them later down the line.

enum class ResourceTag
{
    ModelGeometry,
    ModelTexture,
    ShaderPassIO,
    ShaderPassResource,
    GPUReadback
};

class Resource
{
public:
    Resource(Device::Ref device);
    ~Resource();

    void Tag(ResourceTag tag);

    UInt64 GetSize() const { return mSize; }
    UInt64 GetStride() const { return mStride; }
    ID3D12Resource* GetResource() const { return mResource; }
protected:
    bool mShouldFree;
    Device::Ref mParentDevice;
    ID3D12Resource* mResource = nullptr;
    UInt64 mSize;
    UInt64 mStride;
    Vector<ResourceTag> mTags;

    void CreateResource(D3D12_HEAP_PROPERTIES* heapProps, D3D12_RESOURCE_DESC* resourceDesc, D3D12_RESOURCE_STATES state);
};
