//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-03 23:55:27
//

#include <RHI/Uploader.hpp>
#include <Core/Logger.hpp>
#include <Core/Timer.hpp>

Uploader::Data Uploader::sData;

void Uploader::Init(Device::Ref device, DescriptorHeaps heaps, Queue::Ref queue)
{
    sData.Device = device;
    sData.Heaps = heaps;
    sData.UploadQueue = queue;
}

void Uploader::EnqueueTextureUpload(Vector<UInt8> buffer, Ref<Resource> texture)
{
    sData.TextureRequests++;

    UploadRequest request;
    request.Type = UploadRequestType::TextureToGPU;
    request.Resource = texture;
    
    D3D12_RESOURCE_DESC desc = texture->GetResource()->GetDesc();
    std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> footprints(desc.MipLevels);
    std::vector<UInt32> num_rows(desc.MipLevels);
    std::vector<UInt64> row_sizes(desc.MipLevels);
    uint64_t totalSize = 0;

    sData.Device->GetDevice()->GetCopyableFootprints(&desc, 0, desc.MipLevels, 0, footprints.data(), num_rows.data(), row_sizes.data(), &totalSize);
    request.StagingBuffer = MakeRef<Buffer>(sData.Device, sData.Heaps, totalSize, 0, BufferType::Copy, "Staging Buffer");

    UInt8 *pixels = reinterpret_cast<UInt8*>(buffer.data());    
    UInt8* mapped;
    request.StagingBuffer->Map(0, 0, (void**)&mapped);
    for (int i = 0; i < desc.MipLevels; i++) {
        for (int j = 0; j < num_rows[i]; j++) {
            memcpy(mapped, pixels, row_sizes[i]);
            mapped += footprints[i].Footprint.RowPitch;
            pixels += row_sizes[i];
        }
    }
    request.StagingBuffer->Unmap(0, 0);

    sData.Requests.push_back(request);
}

void Uploader::EnqueueTextureUpload(Image image, Ref<Resource> buffer)
{
    sData.TextureRequests++;

    UploadRequest request;
    request.Type = UploadRequestType::TextureToGPU;
    request.Resource = buffer;
    
    D3D12_RESOURCE_DESC desc = buffer->GetResource()->GetDesc();
    std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> footprints(desc.MipLevels);
    std::vector<UInt32> num_rows(desc.MipLevels);
    std::vector<UInt64> row_sizes(desc.MipLevels);
    uint64_t totalSize = 0;

    sData.Device->GetDevice()->GetCopyableFootprints(&desc, 0, desc.MipLevels, 0, footprints.data(), num_rows.data(), row_sizes.data(), &totalSize);
    request.StagingBuffer = MakeRef<Buffer>(sData.Device, sData.Heaps, totalSize, 0, BufferType::Copy, "Staging Buffer");

    UInt8 *pixels = reinterpret_cast<UInt8*>(image.Pixels.data());    
    UInt8* mapped;
    request.StagingBuffer->Map(0, 0, (void**)&mapped);
    for (int i = 0; i < desc.MipLevels; i++) {
        for (int j = 0; j < num_rows[i]; j++) {
            memcpy(mapped, pixels, row_sizes[i]);
            mapped += footprints[i].Footprint.RowPitch;
            pixels += row_sizes[i];
        }
    }
    request.StagingBuffer->Unmap(0, 0);

    sData.Requests.push_back(request);
}

void Uploader::EnqueueBufferUpload(void* data, UInt64 size, Ref<Resource> buffer)
{
    sData.BufferRequests++;
    UploadRequest request;
    request.Type = UploadRequestType::BufferCPUToGPU;
    request.Resource = buffer;
    request.StagingBuffer = MakeRef<Buffer>(sData.Device, sData.Heaps, size, 0, BufferType::Copy, "Staging Buffer");

    void* mapped;
    request.StagingBuffer->Map(0, 0, &mapped);
    memcpy(mapped, data, size);
    request.StagingBuffer->Unmap(0, 0);

    sData.Requests.push_back(request);
}

void Uploader::Flush()
{
    CommandBuffer::Ref cmdBuffer = MakeRef<CommandBuffer>(sData.Device, sData.UploadQueue, sData.Heaps, true);
    cmdBuffer->Begin();

    LOG_INFO("Flushing {0} upload requests ({1} buffer uploads, {2} texture uploads)", sData.Requests.size(), sData.BufferRequests, sData.TextureRequests);
    for (auto request : sData.Requests) {        
        switch (request.Type) {
            case UploadRequestType::BufferCPUToGPU: {
                cmdBuffer->CopyBufferToBuffer(request.Resource, request.StagingBuffer);
                break;
            }
            case UploadRequestType::TextureToGPU: {
                cmdBuffer->CopyBufferToTexture(request.Resource, request.StagingBuffer);
                break;
            }
        }
    }

    cmdBuffer->End();
    sData.UploadQueue->Submit({ cmdBuffer });
    sData.CmdBuffer = cmdBuffer;
}

void Uploader::ClearRequests()
{
    sData.BufferRequests = 0;
    sData.TextureRequests = 0;
    sData.Requests.clear();
    sData.CmdBuffer.reset();
}
