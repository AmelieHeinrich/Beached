//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-03-14 02:11:19
//

#include "Deferred.hpp"

#include <imgui.h>

Deferred::Deferred(RHI::Ref rhi)
    : RenderPass(rhi)
{
    Asset::Handle computeShader = AssetManager::Get("Assets/Shaders/Deferred/Compute.hlsl", AssetType::Shader);

    auto signature = mRHI->CreateRootSignature({ RootType::PushConstant }, sizeof(int) * 1);
    mPipeline = mRHI->CreateComputePipeline(computeShader->Shader, signature);

    mSampler = mRHI->CreateSampler(SamplerAddress::Wrap, SamplerFilter::Linear, true);
    mClampSampler = mRHI->CreateSampler(SamplerAddress::Clamp, SamplerFilter::Nearest, false);
    mShadowSampler = mRHI->CreateSampler(SamplerAddress::Clamp, SamplerFilter::Nearest, false, 1, true);
}

void Deferred::Render(const Frame& frame, Scene& scene)
{

}

void Deferred::UI(const Frame& frame)
{
    if (ImGui::TreeNodeEx("Deferred", ImGuiTreeNodeFlags_Framed)) {
        ImGui::TreePop();
    }
}
