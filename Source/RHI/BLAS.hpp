//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-12 04:54:59
//

#pragma once

#include <RHI/AccelerationStructure.hpp>

class BLAS : public AccelerationStructure
{
public:
    using Ref = Ref<BLAS>;

    BLAS(Device::Ref device, DescriptorHeaps& heaps, Buffer::Ref vertex, Buffer::Ref index, UInt32 vtxCount, UInt32 idxCount, const String& name = "BLAS");
    ~BLAS() = default;

    void FreeScratch();
    UInt64 Address() const { return mResource->GetAddress(); }
private:
    D3D12_RAYTRACING_GEOMETRY_DESC mGeometryDesc;
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS mInputs;
};