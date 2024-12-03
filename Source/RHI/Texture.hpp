//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-03 10:30:38
//

#pragma once

#include <RHI/Device.hpp>
#include <RHI/Resource.hpp>

enum class TextureFormat
{
    Unknown = DXGI_FORMAT_UNKNOWN,
    RGBA8 = DXGI_FORMAT_R8G8B8A8_UNORM,
    RGBA8_sRGB = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB
};

enum class TextureUsage
{
    RenderTarget = BIT(1),
    DepthTarget = BIT(2),
    Storage = BIT(3),
    ShaderResource = BIT(4)
};

enum class TextureLayout
{
    Common = D3D12_RESOURCE_STATE_COMMON,
    Shader = D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE,
    Storage = D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
    DepthWrite = D3D12_RESOURCE_STATE_DEPTH_WRITE,
    ColorWrite = D3D12_RESOURCE_STATE_RENDER_TARGET,
    CopySource = D3D12_RESOURCE_STATE_COPY_SOURCE,
    CopyDest = D3D12_RESOURCE_STATE_COPY_DEST,
    Present = D3D12_RESOURCE_STATE_PRESENT
};

struct TextureDesc
{
    String Name;
    UInt32 Width;
    UInt32 Height;
    UInt32 Depth;
    UInt32 Levels;
    TextureFormat Format;
    TextureUsage Usage;
};

class Texture : public Resource
{
public:
    using Ref = Ref<Texture>;

    Texture(Device::Ref device, ID3D12Resource* resource, TextureDesc desc);
    Texture(Device::Ref device, TextureDesc desc);
    ~Texture();

    TextureDesc GetDesc() const { return mDesc; }

    TextureLayout GetLayout() const { return mLayout; }
    void SetLayout(TextureLayout layout) { mLayout = layout; }
private:
    TextureDesc mDesc;
    TextureLayout mLayout;
};

inline constexpr bool operator&(TextureUsage x, TextureUsage y)
{
    return (static_cast<UInt32>(x) & static_cast<UInt32>(y) == 0);
}

inline constexpr TextureUsage operator|(TextureUsage x, TextureUsage y)
{
    return static_cast<TextureUsage>(static_cast<UInt32>(x) | static_cast<UInt32>(y));
}

inline constexpr TextureUsage operator^(TextureUsage x, TextureUsage y)
{
    return static_cast<TextureUsage>(static_cast<UInt32>(x) ^ static_cast<UInt32>(y));
}

inline constexpr TextureUsage operator~(TextureUsage x)
{
    return static_cast<TextureUsage>(~static_cast<UInt32>(x));
}

inline TextureUsage& operator|=(TextureUsage & x, TextureUsage y)
{
    x = x | y;
    return x;
}

inline TextureUsage& operator^=(TextureUsage & x, TextureUsage y)
{
    x = x ^ y;
    return x;
}
