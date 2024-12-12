//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-12 04:43:57
//

#include <RHI/AccelerationStructure.hpp>

AccelerationStructure::AccelerationStructure(Device::Ref device, DescriptorHeaps& heaps)
    : mDevice(device), mHeaps(heaps)
{
}

void AccelerationStructure::Allocate(const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS& inputs, UInt64 *scratchSize, const String& name)
{
    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO prebuildInfo = {};
    mDevice->GetDevice()->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &prebuildInfo);
    if (scratchSize)
        *scratchSize = prebuildInfo.UpdateScratchDataSizeInBytes;

    // Create scratch
    mScratch = MakeRef<Buffer>(mDevice, mHeaps, prebuildInfo.ScratchDataSizeInBytes, 0, BufferType::Storage, "Scratch Acceleration Structure " + name);

    // Create AS
    mResource = MakeRef<Buffer>(mDevice, mHeaps, prebuildInfo.ResultDataMaxSizeInBytes, 0, BufferType::AccelerationStructure, name);
}
