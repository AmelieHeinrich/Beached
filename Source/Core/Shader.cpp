//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-03 18:38:56
//

#include <Core/Shader.hpp>
#include <Core/File.hpp>
#include <Core/Logger.hpp>

#include <atlbase.h>
#include <DXC/dxcapi.h>
#include <wrl/client.h>

const char* GetProfileFromType(ShaderType type)
{
    switch (type) {
        case ShaderType::Vertex: {
            return "vs_6_8";
        }
        case ShaderType::Fragment: {
            return "ps_6_8";
        }
        case ShaderType::Compute: {
            return "cs_6_8";
        }
        case ShaderType::Hull: {
            return "hs_6_8";
        }
        case ShaderType::Domain: {
            return "ds_6_8";
        }
        case ShaderType::Mesh: {
            return "ms_6_8";
        }
        case ShaderType::Amplification: {
            return "as_6_8";
        }
        case ShaderType::Library: {
            return "lib_6_8";
        }
    }
    return "???";
}

Shader ShaderCompiler::Compile(const String& path, const String& entry, ShaderType type)
{
    using namespace Microsoft::WRL;
    Shader result = {};

    String source = File::ReadFile(path);

    wchar_t wideTarget[512];
    swprintf_s(wideTarget, 512, L"%hs", GetProfileFromType(type));
    
    wchar_t wideEntry[512];
    swprintf_s(wideEntry, 512, L"%hs", entry.c_str());

    ComPtr<IDxcUtils> pUtils;
    ComPtr<IDxcCompiler> pCompiler;
    if (!SUCCEEDED(DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&pUtils)))) {
        LOG_ERROR("DXC: Failed to create DXC utils instance!");
        return { false };
    }
    if (!SUCCEEDED(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&pCompiler)))) {
        LOG_ERROR("DXC: Failed to create DXC compiler instance!");
        return { false };
    }

    ComPtr<IDxcIncludeHandler> pIncludeHandler;
    if (!SUCCEEDED(pUtils->CreateDefaultIncludeHandler(&pIncludeHandler))) {
        LOG_ERROR("DXC: Failed to create default include handler!");
        return { false };
    }

    ComPtr<IDxcBlobEncoding> pSourceBlob;
    if (!SUCCEEDED(pUtils->CreateBlob(source.c_str(), source.size(), 0, &pSourceBlob))) {
        LOG_ERROR("DXC: Failed to create output blob!");
        return { false };
    }

    LPCWSTR pArgs[] = {
        L"-Zi",
        L"-Fd",
        L"-Fre",
        L"-Qembed_debug",
        L"-Wno-payload-access-perf",
        L"-Wno-payload-access-shader"
    };

    ComPtr<IDxcOperationResult> pResult;
    if (!SUCCEEDED(pCompiler->Compile(pSourceBlob.Get(), L"Shader", wideEntry, wideTarget, pArgs, ARRAYSIZE(pArgs), nullptr, 0, pIncludeHandler.Get(), &pResult))) {
        LOG_ERROR("[DXC] DXC: Failed to compile shader!");
        return { false };
    }

    ComPtr<IDxcBlobEncoding> pErrors;
    pResult->GetErrorBuffer(&pErrors);

    if (pErrors && pErrors->GetBufferSize() != 0) {
        ComPtr<IDxcBlobUtf8> pErrorsU8;
        pErrors->QueryInterface(IID_PPV_ARGS(&pErrorsU8));
        LOG_ERROR("[DXC] Shader errors:%s", (char*)pErrorsU8->GetStringPointer());
        return { false };
    }

    HRESULT Status;
    pResult->GetStatus(&Status);

    ComPtr<IDxcBlob> pShaderBlob;
    pResult->GetResult(&pShaderBlob);

    result.Type = type;
    result.Bytecode.resize(pShaderBlob->GetBufferSize() / sizeof(uint32_t));
    memcpy(result.Bytecode.data(), pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize());

    LOG_DEBUG("Compiled shader {0}", path.c_str());
    return result;
}
