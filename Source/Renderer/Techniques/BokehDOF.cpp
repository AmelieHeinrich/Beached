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

        auto cocSignature = mRHI->CreateRootSignature({ RootType::PushConstant }, sizeof(float) * 6);
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
        // Get resources
        ::Ref<RenderPassIO> depth = PassManager::Get("GBufferDepth");
        ::Ref<RenderPassIO> color = PassManager::Get("MainColorBuffer");
        ::Ref<RenderPassIO> nearTexture = PassManager::Get("NearPlaneCOC");
        ::Ref<RenderPassIO> farTexture = PassManager::Get("FarPlaneCOC");

        struct Data {
            int DepthInput;
            int ColorInput;
            int Sampler;
            int NearOutput;

            int FarOutput;
            float FocalPoint;
            glm::vec2 Pad;
            
            glm::mat4 ProjInv;
        } data = {
            depth->ShaderResourceView->GetDescriptor().Index,
            color->ShaderResourceView->GetDescriptor().Index,
            mPointSampler->BindlesssSampler(),
            nearTexture->UnorderedAccessView->GetDescriptor().Index,

            farTexture->UnorderedAccessView->GetDescriptor().Index,
            mFocalPoint,
            glm::vec2(0.0f),

            glm::inverse(scene.Camera.Projection())
        };

        frame.CommandBuffer->BeginMarker("COC Generation");
        frame.CommandBuffer->SetComputePipeline(mCOCGeneration);
        frame.CommandBuffer->Barrier(depth->Texture, ResourceLayout::Shader);
        frame.CommandBuffer->Barrier(color->Texture, ResourceLayout::Shader);
        frame.CommandBuffer->Barrier(nearTexture->Texture, ResourceLayout::Storage);
        frame.CommandBuffer->Barrier(farTexture->Texture, ResourceLayout::Storage);
        frame.CommandBuffer->ComputePushConstants(&data, sizeof(Data), 0);
        frame.CommandBuffer->Dispatch(std::max(depth->Desc.Width / 7, 1u), std::max(depth->Desc.Height / 7, 1u), 1);
        frame.CommandBuffer->Barrier(depth->Texture, ResourceLayout::DepthWrite);
        frame.CommandBuffer->EndMarker();
    }
    frame.CommandBuffer->EndMarker();
}

void BokehDOF::UI(const Frame& frame)
{
    ::Ref<RenderPassIO> nearTexture = PassManager::Get("NearPlaneCOC");
    ::Ref<RenderPassIO> farTexture = PassManager::Get("FarPlaneCOC");

    if (ImGui::TreeNodeEx("Bokeh Depth of Field", ImGuiTreeNodeFlags_Framed)) {
        ImGui::SliderFloat("Focal Point", &mFocalPoint, CAMERA_NEAR, CAMERA_FAR);
        if (ImGui::TreeNodeEx("Near Field", ImGuiTreeNodeFlags_Framed)) {
            frame.CommandBuffer->Barrier(nearTexture->Texture, ResourceLayout::Shader);
            ImGui::Image((ImTextureID)nearTexture->ShaderResourceView->GetDescriptor().GPU.ptr, ImVec2(256, 256));
            ImGui::TreePop();
        }
        if (ImGui::TreeNodeEx("Far Field", ImGuiTreeNodeFlags_Framed)) {
            frame.CommandBuffer->Barrier(farTexture->Texture, ResourceLayout::Shader);
            ImGui::Image((ImTextureID)farTexture->ShaderResourceView->GetDescriptor().GPU.ptr, ImVec2(256, 256));
            ImGui::TreePop();
        }
        ImGui::TreePop();
    }
}
