//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-03 06:12:55
//

#include <RHI/Utilities.hpp>

void D3DUtils::Release(IUnknown* object)
{
    if (object) {
        object->Release();
        object = nullptr;
    }
}
