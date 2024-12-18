//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-06 00:22:01
//

#include <Renderer/Techniques/Composite.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <ImGui/imgui.h>

Composite::Composite(RHI::Ref rhi)
    : RenderPass(rhi)
{
    Asset::Handle computeShader = AssetManager::Get("Assets/Shaders/Composite/Compute.hlsl", AssetType::Shader);

    mSignature = mRHI->CreateRootSignature({ RootType::PushConstant }, sizeof(int) * 4);
    mPipeline = mRHI->CreateComputePipeline(computeShader->Shader, mSignature);
}

void Composite::Render(const Frame& frame, const Scene& scene)
{
    ::Ref<RenderPassIO> hdr = PassManager::Get("MainColorBuffer");
    ::Ref<RenderPassIO> ldr = PassManager::Get("OutputLDR");

    frame.CommandBuffer->BeginMarker("Composite");
    
    struct {
        int Input;
        int Output;
        float Gamma;
        int Pad;
    } PushConstants = {
        hdr->UnorderedAccessView->GetDescriptor().Index,
        ldr->UnorderedAccessView->GetDescriptor().Index,
        mGamma,
        0
    };

    // Tonemap color buffer
    frame.CommandBuffer->BeginMarker("Tonemap");
    frame.CommandBuffer->Barrier(hdr->Texture, ResourceLayout::Storage);
    frame.CommandBuffer->Barrier(ldr->Texture, ResourceLayout::Storage);
    frame.CommandBuffer->SetComputePipeline(mPipeline);
    frame.CommandBuffer->ComputePushConstants(&PushConstants, sizeof(PushConstants), 0);
    frame.CommandBuffer->Dispatch(hdr->Desc.Width / 8, hdr->Desc.Height / 8, 1);
    frame.CommandBuffer->Barrier(hdr->Texture, ResourceLayout::Common);
    frame.CommandBuffer->UAVBarrier(ldr->Texture);
    frame.CommandBuffer->EndMarker();
    //
    
    // Copy LDR to backbuffer
    frame.CommandBuffer->BeginMarker("Copy to Backbuffer");
    frame.CommandBuffer->Barrier(ldr->Texture, ResourceLayout::CopySource);
    frame.CommandBuffer->Barrier(frame.Backbuffer, ResourceLayout::CopyDest);
    frame.CommandBuffer->CopyTextureToTexture(frame.Backbuffer, ldr->Texture);
    frame.CommandBuffer->Barrier(ldr->Texture, ResourceLayout::Common);
    frame.CommandBuffer->Barrier(frame.Backbuffer, ResourceLayout::ColorWrite);
    frame.CommandBuffer->EndMarker();
    //

    frame.CommandBuffer->EndMarker();
}

void Composite::UI(const Frame& frame)
{
    if (ImGui::TreeNodeEx("Composite", ImGuiTreeNodeFlags_Framed)) {
        ImGui::SliderFloat("Gamma", &mGamma, 0.0f, 5.0f, "%.1f");
        ImGui::TreePop();
    }
}
