//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-03 06:11:18
//

#include <RHI/Device.hpp>
#include <RHI/Utilities.hpp>

#include <Core/Assert.hpp>
#include <Core/Logger.hpp>
#include <Core/UTF.hpp>

#include <unordered_map>

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
    std::unordered_map<IDXGIAdapter1*, UInt64> adapterScores;
    for (UInt32 adapterIndex = 0;; adapterIndex++) {
        IDXGIAdapter1* adapter;
        if (FAILED(mFactory->EnumAdapterByGpuPreference(adapterIndex, DXGI_GPU_PREFERENCE_UNSPECIFIED, IID_PPV_ARGS(&adapter))))
            break;

        adapterScores[adapter] = D3DUtils::CalculateAdapterScore(adapter);
    }

    std::pair<IDXGIAdapter1*, UInt64> bestAdapter = { nullptr, 0 };
    for (auto& pair : adapterScores) {
        DXGI_ADAPTER_DESC1 desc;
        pair.first->GetDesc1(&desc);

        if (pair.second > bestAdapter.second) {
            bestAdapter = pair;
        }
        LOG_DEBUG("Found GPU {0} with score {1}", UTF::WideToAscii(desc.Description), pair.second);
    }
    mAdapter = bestAdapter.first;

    DXGI_ADAPTER_DESC1 desc;
    mAdapter->GetDesc1(&desc);
    LOG_INFO("Selecting GPU {0}", UTF::WideToAscii(desc.Description));
}

Device::~Device()
{
    D3DUtils::Release(mAdapter);
    D3DUtils::Release(mFactory);
}
