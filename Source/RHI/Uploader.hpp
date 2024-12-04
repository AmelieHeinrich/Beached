//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-03 23:48:52
//

#pragma once

#include <Asset/Image.hpp>

#include <RHI/CommandBuffer.hpp>
#include <RHI/Resource.hpp>
#include <RHI/Queue.hpp>
#include <RHI/Buffer.hpp>

class Uploader
{
public:
    static void Init(Device::Ref device, DescriptorHeaps heaps, Queue::Ref queue);
    static void EnqueueTextureUpload(Vector<UInt8> buffer, Ref<Resource> texture);
    static void EnqueueTextureUpload(Image image, Ref<Resource> buffer);
    static void EnqueueBufferUpload(void* data, UInt64 size, Ref<Resource> buffer);
    static void Flush();
    static void ClearRequests();

private:
    enum class UploadRequestType
    {
        BufferCPUToGPU,
        TextureToGPU
    };

    struct UploadRequest
    {
        UploadRequestType Type;
        Ref<Resource> Resource;
        Ref<Buffer> StagingBuffer;
        UInt64 Size;
    };

    static struct Data
    {
        DescriptorHeaps Heaps;
        Device::Ref Device;
        Queue::Ref UploadQueue;
        CommandBuffer::Ref CmdBuffer;
        Vector<UploadRequest> Requests;
    } sData;
};
