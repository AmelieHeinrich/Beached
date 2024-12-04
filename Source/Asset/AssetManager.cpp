//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-04 21:02:03
//

#include <Asset/AssetManager.hpp>
#include <Core/Logger.hpp>
#include <RHI/Uploader.hpp>

AssetManager::Data AssetManager::sData;

void AssetManager::Init(RHI::Ref rhi)
{
    sData.mRHI = rhi;
}

void AssetManager::Clean()
{
    sData.mAssets.clear();
}

Asset::Handle AssetManager::Get(const String& path, ResourceType type)
{
    if (sData.mAssets.count(path) > 0) {
        sData.mAssets[path]->RefCount++;
        return sData.mAssets[path];
    }

    Asset::Handle asset = MakeRef<Asset>();
    asset->RefCount++;
    asset->Type = type;
    asset->Path = path;

    switch (type) {
        case ResourceType::GLTF: {
            asset->Model.Load(sData.mRHI, path);
            break;
        }
        case ResourceType::Texture: {
            Image image;
            image.Load(path);

            TextureDesc desc;
            desc.Width = image.Width;
            desc.Height = image.Height;
            desc.Levels = image.Levels;
            desc.Depth = 1;
            desc.Name = path;
            desc.Format = image.Compressed ? TextureFormat::RGBA8 : TextureFormat::RGBA8;
            desc.Usage = TextureUsage::ShaderResource;
            asset->Texture = sData.mRHI->CreateTexture(desc);

            Uploader::EnqueueTextureUpload(image, asset->Texture);
            break;
        }
    }

    sData.mAssets[path] = asset;
    return asset;
}

void AssetManager::Free(Asset::Handle handle)
{
    sData.mAssets[handle->Path]->RefCount--;
    if (sData.mAssets[handle->Path]->RefCount == 0) {
        sData.mAssets[handle->Path].reset();
        sData.mAssets.erase(handle->Path);
    }
}
