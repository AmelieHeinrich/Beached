//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2025-01-18 17:36:13
//

#include <Renderer/Techniques/AutoExposure.hpp>

#include <imgui.h>

AutoExposure::AutoExposure(RHI::Ref rhi)
    : RenderPass(rhi)
{
    // Histogram shader
    {
        mLuminanceHistogram = mRHI->CreateBuffer(256 * sizeof(UInt32), 0, BufferType::Storage, "Luminance Histogram");

        Asset::Handle shader = AssetManager::Get("Assets/Shaders/AutoExposure/HistogramCompute.hlsl", AssetType::Shader);

        auto signature = mRHI->CreateRootSignature({ RootType::PushConstant }, sizeof(glm::vec4));
        mHistogramShader = mRHI->CreateComputePipeline(shader->Shader, signature);
    }
}

void AutoExposure::Render(const Frame& frame, Scene& scene)
{
    // Histogram pass
    frame.CommandBuffer->BeginMarker("Luminance Adaptive Auto-Exposure");
    {
        ::Ref<RenderPassIO> sourceBuffer = PassManager::Get("MainColorBuffer");

        struct Data {
            int Input;
            int Output;
            float MinLogLuminance;
            float MinLogLuminanceInv;
        } data = {
            sourceBuffer->ShaderResourceView->GetDescriptor().Index,
            mLuminanceHistogram->UAV(),
            mLogLuminance,
            1.0f / mLogLuminance
        };

        frame.CommandBuffer->BeginMarker("Build Histogram");
        frame.CommandBuffer->SetComputePipeline(mHistogramShader);
        frame.CommandBuffer->ComputePushConstants(&data, sizeof(data), 0);
        frame.CommandBuffer->Dispatch(std::max(sourceBuffer->Desc.Width / 16, 1u), std::max(sourceBuffer->Desc.Height / 16, 1u), 1);
        frame.CommandBuffer->EndMarker();
    }
    frame.CommandBuffer->EndMarker();
}

void AutoExposure::UI(const Frame& frame)
{
    if (ImGui::TreeNodeEx("Luminance Adaptive Auto-Exposure", ImGuiTreeNodeFlags_Framed)) {
        ImGui::SliderFloat("Log Luminance", &mLogLuminance, 1.0f, 30.0f, "%.1f");
        ImGui::TreePop();
    }
}
