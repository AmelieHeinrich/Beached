//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-12 05:05:49
//

#pragma once

#include <RHI/AccelerationStructure.hpp>

class TLAS : public AccelerationStructure
{
public:
    using Ref = Ref<TLAS>;

    TLAS(Device::Ref device, DescriptorHeaps& heaps, Buffer::Ref instanceBuffer, UInt32 numInstance, const String& name = "TLAS");
    ~TLAS();

    UInt32 Bindless() const { return mSRV.Index; }
private:
    DescriptorHeap::Descriptor mSRV;

    UInt64 mScratchSize;
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS mInputs;
};
