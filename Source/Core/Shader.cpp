//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-03 18:38:56
//

#include <Core/Shader.hpp>
#include <Core/File.hpp>
#include <Core/Logger.hpp>
#include <RHI/Utilities.hpp>

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
    Shader result = {};

    String wrappedSource = File::ReadFile(path); 
    const char* source = wrappedSource.c_str();

    wchar_t wideTarget[512];
    swprintf_s(wideTarget, 512, L"%hs", GetProfileFromType(type));
    
    wchar_t wideEntry[512];
    swprintf_s(wideEntry, 512, L"%hs", entry.c_str());

    IDxcUtils* pUtils = nullptr;
    IDxcCompiler* pCompiler = nullptr;
    if (!SUCCEEDED(DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&pUtils)))) {
        LOG_ERROR("DXC: Failed to create DXC utils instance!");
        return { false };
    }
    if (!SUCCEEDED(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&pCompiler)))) {
        LOG_ERROR("DXC: Failed to create DXC compiler instance!");
        return { false };
    }

    IDxcIncludeHandler* pIncludeHandler = nullptr;
    if (!SUCCEEDED(pUtils->CreateDefaultIncludeHandler(&pIncludeHandler))) {
        LOG_ERROR("DXC: Failed to create default include handler!");
        return { false };
    }

    IDxcBlobEncoding* pSourceBlob = nullptr;
    if (!SUCCEEDED(pUtils->CreateBlob(source, wrappedSource.size(), 0, &pSourceBlob))) {
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

    IDxcOperationResult* pResult = nullptr;
    if (!SUCCEEDED(pCompiler->Compile(pSourceBlob, L"Shader", wideEntry, wideTarget, pArgs, ARRAYSIZE(pArgs), nullptr, 0, pIncludeHandler, &pResult))) {
        LOG_ERROR("[DXC] DXC: Failed to compile shader!");
        return { false };
    }

    IDxcBlobEncoding* pErrors = nullptr;
    pResult->GetErrorBuffer(&pErrors);

    if (pErrors && pErrors->GetBufferSize() != 0) {
        IDxcBlobUtf8* pErrorsU8 = nullptr;
        pErrors->QueryInterface(IID_PPV_ARGS(&pErrorsU8));
        LOG_ERROR("[DXC] Shader errors: {0}", (char*)pErrorsU8->GetStringPointer());
        pErrorsU8->Release();
        pErrors->Release();
        return { false };
    }

    HRESULT Status;
    pResult->GetStatus(&Status);

    IDxcBlob* pShaderBlob = nullptr;
    pResult->GetResult(&pShaderBlob);

    result.Type = type;
    result.Bytecode.resize(pShaderBlob->GetBufferSize());
    memcpy(result.Bytecode.data(), pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize());
    LOG_DEBUG("Compiled shader {0}", path.c_str());

    D3DUtils::Release(pShaderBlob);
    D3DUtils::Release(pErrors);
    D3DUtils::Release(pResult);
    D3DUtils::Release(pSourceBlob);
    D3DUtils::Release(pIncludeHandler);
    D3DUtils::Release(pCompiler);
    D3DUtils::Release(pUtils);
    return result;
}
