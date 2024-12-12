//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-12 04:40:23
//

#pragma once

#include <RHI/Device.hpp>
#include <RHI/DescriptorHeap.hpp>
#include <RHI/Buffer.hpp>

class AccelerationStructure
{
public:
    AccelerationStructure(Device::Ref device, DescriptorHeaps& heaps);
    ~AccelerationStructure() = default;

    void FreeScratch();
protected:
    void Allocate(const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS& inputs, UInt64 *scratchSize = nullptr, const String& name = "Acceleration Structure");

    Device::Ref mDevice;
    DescriptorHeaps mHeaps;

    Buffer::Ref mResource;
    Buffer::Ref mScratch;
};
