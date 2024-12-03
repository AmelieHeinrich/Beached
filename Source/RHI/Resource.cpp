//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-03 10:44:01
//

#include <RHI/Resource.hpp>
#include <RHI/Utilities.hpp>

#include <Core/UTF.hpp>
#include <Core/Assert.hpp>

Resource::Resource(Device::Ref device)
    : mParentDevice(device), mResource(nullptr), mSize(0), mStride(0), mShouldFree(true), mLayout(ResourceLayout::Common)
{
}

Resource::~Resource()
{
    if (mShouldFree)
        D3DUtils::Release(mResource);
}

void Resource::SetName(const String& string)
{
    mResource->SetName(UTF::AsciiToWide(string).data());
}

void Resource::Tag(ResourceTag tag)
{
    mTags.push_back(tag);
}

void Resource::CreateResource(D3D12_HEAP_PROPERTIES* heapProps, D3D12_RESOURCE_DESC* resourceDesc, D3D12_RESOURCE_STATES state)
{
    HRESULT result = mParentDevice->GetDevice()->CreateCommittedResource(heapProps, D3D12_HEAP_FLAG_NONE, resourceDesc, state, nullptr, IID_PPV_ARGS(&mResource));
    ASSERT(SUCCEEDED(result), "Failed to allocate resource!");

    D3D12_RESOURCE_ALLOCATION_INFO info = {};
    mParentDevice->GetDevice()->GetResourceAllocationInfo(0, 1, resourceDesc);
    mSize = info.SizeInBytes;
}
