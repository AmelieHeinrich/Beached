//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2025-01-11 06:46:05
//

#include <Renderer/Techniques/BokehDOF.hpp>

#include <imgui.h>

BokehDOF::BokehDOF(RHI::Ref rhi)
    : RenderPass(rhi)
{
    // COC Compute
    {
        Asset::Handle cocShader = AssetManager::Get("Assets/Shaders/BokehDOF/COCCompute.hlsl", AssetType::Shader);

        auto cocSignature = mRHI->CreateRootSignature({ RootType::PushConstant }, sizeof(float) * 8 + sizeof(glm::mat4));
        mCOCGeneration = mRHI->CreateComputePipeline(cocShader->Shader, cocSignature);
    }

    // Samplers
    mPointSampler = mRHI->CreateSampler(SamplerAddress::Wrap, SamplerFilter::Nearest);
}

void BokehDOF::Render(const Frame& frame, Scene& scene)
{
    // Generate COC textures
    frame.CommandBuffer->BeginMarker("Bokeh DOF");
    {
        frame.CommandBuffer->BeginMarker("COC Generation");
        frame.CommandBuffer->EndMarker();
    }
    frame.CommandBuffer->EndMarker();
}

void BokehDOF::UI(const Frame& frame)
{
    if (ImGui::TreeNodeEx("Bokeh Depth of Field", ImGuiTreeNodeFlags_Framed)) {
        ImGui::SliderFloat("Focal Point", &mFocalPoint, CAMERA_NEAR, CAMERA_FAR);
        ImGui::TreePop();
    }
}
