//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-03 06:22:48
//

#pragma once

#include <RHI/Device.hpp>

class RHI
{
public:
    using Ref = Ref<RHI>;

    RHI();
    ~RHI();
private:
    Device::Ref mDevice;
};
