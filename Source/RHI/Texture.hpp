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
private:
    TextureDesc mDesc;
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
