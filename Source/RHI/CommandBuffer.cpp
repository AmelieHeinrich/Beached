//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-03 11:56:02
//

#include <RHI/CommandBuffer.hpp>
#include <RHI/Utilities.hpp>
#include <Core/Assert.hpp>

#include <imgui.h>
#include <imgui_impl_dx12.h>
#include <imgui_impl_win32.h>

CommandBuffer::CommandBuffer(Device::Ref device, Queue::Ref queue, DescriptorHeaps heaps, bool singleTime)
    : mSingleTime(singleTime), mParentQueue(queue), mHeaps(heaps)
{
    HRESULT result = device->GetDevice()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE(queue->GetType()), IID_PPV_ARGS(&mAllocator));
    ASSERT(SUCCEEDED(result), "Failed to create command allocator!");

    result = device->GetDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE(queue->GetType()), mAllocator, nullptr, IID_PPV_ARGS(&mList));
    ASSERT(SUCCEEDED(result), "Failed to create command list!");

    if (!singleTime) {
        mList->Close();
    }
}

CommandBuffer::~CommandBuffer()
{
    D3DUtils::Release(mAllocator);
    D3DUtils::Release(mList);
}

void CommandBuffer::Begin()
{
    if (!mSingleTime) {
        mAllocator->Reset();
        mList->Reset(mAllocator, nullptr);
    }

    ID3D12DescriptorHeap* heaps[] = {
        mHeaps[DescriptorHeapType::ShaderResource]->GetHeap(),
        mHeaps[DescriptorHeapType::Sampler]->GetHeap()
    };
    mList->SetDescriptorHeaps(2, heaps);
}

void CommandBuffer::Barrier(Texture::Ref texture, ResourceLayout layout, UInt32 mip)
{
    D3D12_RESOURCE_BARRIER Barrier = {};
    Barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    Barrier.Transition.pResource = texture->GetResource();
    Barrier.Transition.StateBefore = D3D12_RESOURCE_STATES(texture->GetLayout());
    Barrier.Transition.StateAfter = D3D12_RESOURCE_STATES(layout);
    Barrier.Transition.Subresource = mip == VIEW_ALL_MIPS ? D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES : mip;
    
    if (Barrier.Transition.StateBefore == D3D12_RESOURCE_STATE_UNORDERED_ACCESS && Barrier.Transition.StateAfter == D3D12_RESOURCE_STATE_UNORDERED_ACCESS) {
        Barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
        Barrier.UAV.pResource = texture->GetResource();
    } else {
        if (Barrier.Transition.StateBefore == Barrier.Transition.StateAfter)
            return;
    }
    
    mList->ResourceBarrier(1, &Barrier);
    texture->SetLayout(layout);
}

void CommandBuffer::SetViewport(float x, float y, float width, float height)
{
    D3D12_VIEWPORT Viewport = {};
    Viewport.Width = width;
    Viewport.Height = height;
    Viewport.MinDepth = 0.0f;
    Viewport.MaxDepth = 1.0f;
    Viewport.TopLeftX = x;
    Viewport.TopLeftY = y;

    D3D12_RECT Rect = {};
    Rect.right = width;
    Rect.bottom = height;
    Rect.top = 0.0f;
    Rect.left = 0.0f;

    if (Rect.right < 0 || Rect.bottom < 0)
        return;

    mList->RSSetViewports(1, &Viewport);
    mList->RSSetScissorRects(1, &Rect);
}

void CommandBuffer::SetTopology(Topology topology)
{
    mList->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY(topology));
}

void CommandBuffer::SetGraphicsPipeline(GraphicsPipeline::Ref pipeline)
{
    mList->SetPipelineState(pipeline->GetPipeline());
    mList->SetGraphicsRootSignature(pipeline->GetRootSignature()->GetSignature());
}

void CommandBuffer::SetRenderTargets(const Vector<View::Ref> targets, View::Ref depth)
{
    std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> cpus;
    for (auto& target : targets) {
        cpus.push_back(target->GetDescriptor().CPU);
    }
    D3D12_CPU_DESCRIPTOR_HANDLE depth_cpu;
    if (depth) depth_cpu = depth->GetDescriptor().CPU;

    mList->OMSetRenderTargets(cpus.size(), cpus.data(), false, depth ? &depth_cpu : nullptr);
}

void CommandBuffer::SetVertexBuffer(Buffer::Ref buffer)
{
    mList->IASetVertexBuffers(0, 1, &buffer->mVBV);
}

void CommandBuffer::SetIndexBuffer(Buffer::Ref buffer)
{
    mList->IASetIndexBuffer(&buffer->mIBV);
}

void CommandBuffer::GraphicsPushConstants(const void *data, UInt32 size, int index)
{
    mList->SetGraphicsRoot32BitConstants(index, size / 4, data, 0);
}

void CommandBuffer::ClearRenderTarget(View::Ref view, float r, float g, float b)
{
    float clear[] = { r, g, b, 1.0f };
    mList->ClearRenderTargetView(view->GetDescriptor().CPU, clear, 0, nullptr);
}

void CommandBuffer::Draw(int vertexCount)
{
    mList->DrawInstanced(vertexCount, 1, 0, 0);
}

void CommandBuffer::DrawIndexed(int indexCount)
{
    mList->DrawIndexedInstanced(indexCount, 1, 0, 0, 0);
}

void CommandBuffer::CopyBufferToBuffer(::Ref<Resource> dst, ::Ref<Resource> src)
{
    mList->CopyResource(dst->GetResource(), src->GetResource());
}

void CommandBuffer::End()
{
    mList->Close();
}

void CommandBuffer::BeginGUI(int width, int height)
{
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize.x = width;
    io.DisplaySize.y = height;

    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

void CommandBuffer::EndGUI()
{
    ImGuiIO& io = ImGui::GetIO();

    ID3D12DescriptorHeap* pHeaps[] = { mHeaps[DescriptorHeapType::ShaderResource]->GetHeap(), mHeaps[DescriptorHeapType::Sampler]->GetHeap() };
    mList->SetDescriptorHeaps(2, pHeaps);

    ImGui::Render();
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), mList);
}