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
private:
    IDXGIFactory6* mFactory;
    IDXGIAdapter1* mAdapter;
};
