//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-03 11:52:45
//

#pragma once

#include <RHI/Queue.hpp>
#include <RHI/View.hpp>
#include <RHI/GraphicsPipeline.hpp>

enum class Topology
{
    LineList = D3D_PRIMITIVE_TOPOLOGY_LINELIST,
    LineStrip = D3D_PRIMITIVE_TOPOLOGY_LINESTRIP,
    PointList = D3D_PRIMITIVE_TOPOLOGY_POINTLIST,
    TriangleList = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
    TriangleStrip = D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP
};

class CommandBuffer
{
public:
    using Ref = Ref<CommandBuffer>;
 
    CommandBuffer(Device::Ref device, Queue::Ref queue, DescriptorHeaps heaps, bool singleTime = false);
    ~CommandBuffer();

    void Begin();
    void End();

    void Barrier(Texture::Ref texture, ResourceLayout layout, UInt32 mip = VIEW_ALL_MIPS);
    
    void SetViewport(float x, float y, float width, float height);
    void SetTopology(Topology topology);
    void SetGraphicsPipeline(GraphicsPipeline::Ref pipeline);
    void SetRenderTargets(const Vector<View::Ref> targets, View::Ref depth);
    
    void ClearRenderTarget(View::Ref view, float r, float g, float b);

    void Draw(int vertexCount);

    void BeginGUI(int width, int height);
    void EndGUI();

    ID3D12GraphicsCommandList10* GetList() { return mList; }
    operator ID3D12CommandList*() { return mList; }
private:
    bool mSingleTime;
    Queue::Ref mParentQueue;
    DescriptorHeaps mHeaps;
    ID3D12CommandAllocator* mAllocator = nullptr;
    ID3D12GraphicsCommandList10* mList = nullptr;
};
