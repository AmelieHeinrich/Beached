//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-03 23:55:27
//

#include <RHI/Uploader.hpp>

Uploader::Data Uploader::sData;

void Uploader::Init(Device::Ref device, DescriptorHeaps heaps, Queue::Ref queue)
{
    sData.Device = device;
    sData.Heaps = heaps;
    sData.UploadQueue = queue;
}

void Uploader::EnqueueTextureUpload(void* data, Image image, Ref<Resource> buffer)
{
    UploadRequest request;
    request.Type = UploadRequestType::TextureToGPU;
    request.Resource = buffer;
    if (!image.Compressed) {
        request.StagingBuffer = MakeRef<Buffer>(sData.Device, sData.Heaps, image.Width * image.Height * 4, 0, BufferType::Copy, "Staging Buffer");
    
        void* mapped;
        request.StagingBuffer->Map(0, 0, &mapped);
        memcpy(mapped, data, image.Width * image.Height * 4);
        request.StagingBuffer->Unmap(0, 0);
    }

    sData.Requests.push_back(request);
}

void Uploader::EnqueueBufferUpload(void* data, UInt64 size, Ref<Resource> buffer)
{
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
    sData.Requests.clear();
    sData.CmdBuffer.reset();
}
