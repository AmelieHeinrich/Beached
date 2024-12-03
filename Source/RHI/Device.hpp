//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-03 06:10:13
//

#pragma once

#include <Agility/d3d12.h>
#include <dxgi1_6.h>

#include <Core/Common.hpp>

class Device
{
public:
    using Ref = Ref<Device>;

    Device();
    ~Device();

    ID3D12Device* GetDevice() { return mDevice; }
    IDXGIFactory6* GetFactory() { return mFactory; }
private:
    IDXGIFactory6* mFactory;
    IDXGIAdapter1* mAdapter;
    ID3D12Device* mDevice;
    ID3D12Debug1* mDebug;
};
