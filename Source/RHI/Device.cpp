//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-03 06:11:18
//

#include <RHI/Device.hpp>
#include <RHI/Utilities.hpp>

#include <Core/Assert.hpp>
#include <Core/Logger.hpp>
#include <Core/UTF.hpp>

extern "C"
{
    __declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;

    __declspec(dllexport) extern const uint32_t D3D12SDKVersion = 614;
    __declspec(dllexport) extern const char* D3D12SDKPath = ".\\D3D12\\";
}

Device::Device()
{
    // Create factory
    IDXGIFactory1* tempFactory;
    HRESULT result = CreateDXGIFactory1(IID_PPV_ARGS(&tempFactory));
    ASSERT(SUCCEEDED(result), "Failed to create DXGI factory!");
    tempFactory->QueryInterface(IID_PPV_ARGS(&mFactory));
    tempFactory->Release();

    // Get adapter.
    // TODO(amelie): D3DUtils::CalculateAdapterScore
    IDXGIAdapter1* adapter = nullptr;
    for (UInt32 adapterIndex = 0; SUCCEEDED(mFactory->EnumAdapterByGpuPreference(adapterIndex, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter))); adapterIndex++) {
        DXGI_ADAPTER_DESC1 desc;
        adapter->GetDesc1(&desc);

        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            continue;
        
        if (SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device*), nullptr)))
            break;
    }
    if (!adapter) {
        for (UInt32 adapterIndex = 0; SUCCEEDED(mFactory->EnumAdapters1(adapterIndex, &adapter)); adapterIndex++) {
            DXGI_ADAPTER_DESC1 desc;
            adapter->GetDesc1(&desc);

            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
                continue;

            if (SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device*), nullptr)))
                break;
        }
    }
    ASSERT(adapter, "Failed to find suitable Direct3D 12 adapter.");
    mAdapter = adapter;

    DXGI_ADAPTER_DESC1 desc;
    mAdapter->GetDesc1(&desc);

    LOG_INFO("Selecting GPU {0}", UTF::WideToAscii(desc.Description));
}

Device::~Device()
{
    D3DUtils::Release(mAdapter);
    D3DUtils::Release(mFactory);
}
