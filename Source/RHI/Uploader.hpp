//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-03 23:48:52
//

#pragma once

#include <Asset/Image.hpp>

#include <RHI/RHI.hpp>
#include <RHI/CommandBuffer.hpp>
#include <RHI/Resource.hpp>
#include <RHI/Queue.hpp>
#include <RHI/Buffer.hpp>
#include <RHI/AccelerationStructure.hpp>
#include <RHI/RHI.hpp>

#define KILOBYTES(s) s * 1024
#define MEGABYTES(s) KILOBYTES(s) * 1024
#define GIGABYTES(s) MEGABYTES(s) * 1024

class Uploader
{
public:
    static void Init(RHI* rhi, Device::Ref device, DescriptorHeaps heaps, Queue::Ref queue);
    static void EnqueueTextureUpload(Vector<UInt8> buffer, Ref<Resource> texture);
    static void EnqueueTextureUpload(Image image, Ref<Resource> buffer);
    static void EnqueueBufferUpload(void* data, UInt64 size, Ref<Resource> buffer);
    static void EnqueueAccelerationStructureBuild(Ref<AccelerationStructure> as);
    static void Flush();
    static void ClearRequests();

private:
    static constexpr UInt64 MAX_UPLOAD_BATCH_SIZE = MEGABYTES(512);

    enum class UploadRequestType
    {
        BufferCPUToGPU,
        TextureToGPU,
        BuildAS
    };

    struct UploadRequest
    {
        UploadRequestType Type;

        Ref<Resource> Resource = nullptr;
        Ref<Buffer> StagingBuffer = nullptr;
        Ref<AccelerationStructure> Acceleration = nullptr;
    };

    static struct Data
    {
        RHI* Rhi;
        DescriptorHeaps Heaps;
        Device::Ref Device;
        Queue::Ref UploadQueue;
        CommandBuffer::Ref CmdBuffer;
        Vector<UploadRequest> Requests;

        int TextureRequests;
        int BufferRequests;
        int ASRequests;
        int UploadBatchSize;
    } sData;
};
