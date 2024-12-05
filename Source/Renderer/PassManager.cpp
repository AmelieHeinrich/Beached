//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-05 23:23:58
//

#include <Renderer/PassManager.hpp>
#include <Core/Logger.hpp>
#include <TOML++/toml.hpp>

#include <iostream>

UnorderedMap<String, Ref<RenderPassIO>> PassManager::sPassIOs;

TextureFormat StringToFormat(const String& format)
{
    if (format == "RGBA8")
        return TextureFormat::RGBA8;
    if (format == "D32")
        return TextureFormat::Depth32;
    return TextureFormat::Unknown;
}

void PassManager::Init(RHI::Ref rhi, Window::Ref window)
{
    int width, height;
    window->PollSize(width, height);

    try {
        toml::table tbl = toml::parse_file("Assets/PassResources.toml");
        
        for (auto& tables : tbl) {
            Ref<RenderPassIO> io = MakeRef<RenderPassIO>();
            
            toml::table info = *tables.second.as_table();

            String textureName = tables.first.data();
            String usage = info["Usage"].as_string()->get();
            String format = info["Format"].as_string()->get();

            Int64 texWidth = info["Width"].as_integer()->get();
            Int64 texHeight = info["Height"].as_integer()->get();

            io->Desc.Name = textureName;
            io->Desc.Width = texWidth == 0 ? width : texWidth;
            io->Desc.Height = texHeight == 0 ? height : texHeight;
            io->Desc.Depth = 1;
            io->Desc.Levels = 1;
            io->Desc.Format = StringToFormat(format);
            if (usage == "Render") {
                io->Desc.Usage = TextureUsage::RenderTarget | TextureUsage::Storage | TextureUsage::ShaderResource;
            } else {
                io->Desc.Usage = TextureUsage::DepthTarget | TextureUsage::ShaderResource;
            }

            LOG_INFO("Creating PassIO {0} (Width = {1}, Height = {2}, Format = {3}, Usage = {4})", textureName, io->Desc.Width, io->Desc.Height, format, usage);
            io->Texture = rhi->CreateTexture(io->Desc);

             if (usage == "Render") {
                io->RenderTargetView = rhi->CreateView(io->Texture, ViewType::RenderTarget);
                io->ShaderResourceView = rhi->CreateView(io->Texture, ViewType::ShaderResource);
                io->UnorderedAccessView = rhi->CreateView(io->Texture, ViewType::Storage);
             } else {
                io->DepthTargetView = rhi->CreateView(io->Texture, ViewType::DepthTarget);
             }

            sPassIOs[textureName] = io;
        }
    } catch (const toml::parse_error& err) {
        LOG_CRITICAL("TOML ERROR: {0}", err.what());
    }
}
