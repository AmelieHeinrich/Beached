//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-05 23:23:58
//

#include <Renderer/PassManager.hpp>
#include <RHI/Uploader.hpp>
#include <Core/Logger.hpp>
#include <Asset/Image.hpp>
#include <TOML++/toml.hpp>

UnorderedMap<String, Ref<RenderPassIO>> PassManager::sPassIOs;

void PassManager::Init(RHI::Ref rhi, Window::Ref window)
{
    int width, height;
    window->PollSize(width, height);

    try {
        toml::table tbl = toml::parse_file("Assets/PassResources.toml");
        
        for (auto& tables : tbl) {
            Ref<RenderPassIO> io = MakeRef<RenderPassIO>();
            
            toml::table info = *tables.second.as_table();

            String name = tables.first.data();
            String type = info["Type"].as_string()->get();
            if (type == "Texture") {
                String usage = info["Usage"].as_string()->get();
                String format = info["Format"].as_string()->get();

                Int64 texWidth = info["Width"].as_integer()->get();
                Int64 texHeight = info["Height"].as_integer()->get();

                io->Desc.Name = name;
                io->Desc.Width = texWidth == 0 ? width : texWidth;
                io->Desc.Height = texHeight == 0 ? height : texHeight;
                io->Desc.Depth = 1;
                io->Desc.Levels = 1;
                io->Desc.Format = Texture::StringToFormat(format);
                if (usage == "Render") {
                    io->Desc.Usage = TextureUsage::RenderTarget | TextureUsage::Storage | TextureUsage::ShaderResource;
                } else {
                    io->Desc.Usage = TextureUsage::DepthTarget | TextureUsage::ShaderResource;
                }

                if (info.contains("Downsample")) {
                    Int64 downsample = info["Downsample"].as_integer()->get();
                    io->Desc.Width = std::round(io->Desc.Width / (downsample / 2));
                    io->Desc.Height = std::round(io->Desc.Height / (downsample / 2));
                }

                LOG_INFO("Creating PassIO {0} (Width = {1}, Height = {2}, Format = {3}, Usage = {4})", name, io->Desc.Width, io->Desc.Height, format, usage);
                io->Texture = rhi->CreateTexture(io->Desc);

                if (usage == "Render") {
                    io->RenderTargetView = rhi->CreateView(io->Texture, ViewType::RenderTarget);
                    io->ShaderResourceView = rhi->CreateView(io->Texture, ViewType::ShaderResource);
                    io->UnorderedAccessView = rhi->CreateView(io->Texture, ViewType::Storage);
                } else {
                    io->DepthTargetView = rhi->CreateView(io->Texture, ViewType::DepthTarget);
                    io->ShaderResourceView = rhi->CreateView(io->Texture, ViewType::ShaderResource, ViewDimension::Texture, TextureFormat::R32Float);
                }
            } else if (type == "RingBuffer") {
                Int64 size = info["Size"].as_integer()->get();

                for (int i = 0; i < FRAMES_IN_FLIGHT; i++) {
                    io->RingBuffer[i] = rhi->CreateBuffer(size, 0, BufferType::Constant, name);
                    io->RingBuffer[i]->BuildCBV();
                }
                LOG_INFO("Creating PassIO {0} (Size = {1})", name, size);
            }

            sPassIOs[name] = io;
        }
    } catch (const toml::parse_error& err) {
        LOG_CRITICAL("TOML ERROR: {0}", err.what());
    }

    // Create white and black image
    {
        // White
        {
            Image image;
            image.Width = 1;
            image.Height = 1;
            image.Levels = 1;

            // 0xFFFFFFFF
            image.Pixels.push_back(255);
            image.Pixels.push_back(255);
            image.Pixels.push_back(255);
            image.Pixels.push_back(255);

            TextureDesc desc;
            desc.Width = 1;
            desc.Height = 1;
            desc.Levels = 1;
            desc.Format = TextureFormat::RGBA8;
            desc.Name = "White Texture";
            desc.Usage = TextureUsage::ShaderResource;
            desc.Depth = 1;

            Ref<RenderPassIO> whiteTexture = MakeRef<RenderPassIO>();
            whiteTexture->Desc = desc;
            whiteTexture->Texture = rhi->CreateTexture(desc);
            whiteTexture->ShaderResourceView = rhi->CreateView(whiteTexture->Texture, ViewType::ShaderResource);
            sPassIOs["WhiteTexture"] = whiteTexture;

            Uploader::EnqueueTextureUpload(image, whiteTexture->Texture);
        }
    
        // White
        {
            Image image;
            image.Width = 1;
            image.Height = 1;
            image.Levels = 1;

            // 0xFFFFFFFF
            image.Pixels.push_back(0);
            image.Pixels.push_back(0);
            image.Pixels.push_back(0);
            image.Pixels.push_back(255);

            TextureDesc desc;
            desc.Width = 1;
            desc.Height = 1;
            desc.Levels = 1;
            desc.Format = TextureFormat::RGBA8;
            desc.Name = "Black Texture";
            desc.Usage = TextureUsage::ShaderResource;
            desc.Depth = 1;

            Ref<RenderPassIO> blackTexture = MakeRef<RenderPassIO>();
            blackTexture->Desc = desc;
            blackTexture->Texture = rhi->CreateTexture(desc);
            blackTexture->ShaderResourceView = rhi->CreateView(blackTexture->Texture, ViewType::ShaderResource);
            sPassIOs["BlackTexture"] = blackTexture;

            Uploader::EnqueueTextureUpload(image, blackTexture->Texture);
        }
    }
}
