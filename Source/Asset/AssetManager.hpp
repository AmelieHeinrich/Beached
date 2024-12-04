//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-04 20:57:18
//

#pragma once

#include <Asset/GLTF.hpp>
#include <Asset/Image.hpp>

#include <RHI/RHI.hpp>

enum class ResourceType
{
    GLTF,
    Texture,
    EnvironmentMap
};

struct Asset
{
    String Path;
    ResourceType Type;

    GLTF Model;
    Texture::Ref Texture;

    UInt32 RefCount;

    using Handle = Ref<Asset>;
};

class AssetManager
{
public:
    static void Init(RHI::Ref rhi);
    static void Clean();

    static Asset::Handle Get(const String& path, ResourceType type);
    static void Free(Asset::Handle handle);
private:
    static struct Data
    {
        RHI::Ref mRHI;
        UnorderedMap<String, Asset::Handle> mAssets;
    } sData;
};
