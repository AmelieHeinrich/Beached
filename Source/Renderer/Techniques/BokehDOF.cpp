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

        auto cocSignature = mRHI->CreateRootSignature({ RootType::PushConstant }, sizeof(glm::vec4) * 3);
        mCOCGeneration = mRHI->CreateComputePipeline(cocShader->Shader, cocSignature);
    }

    // Samplers
    mPointClampSampler = mRHI->CreateSampler(SamplerAddress::Clamp, SamplerFilter::Nearest);
}

void BokehDOF::Render(const Frame& frame, Scene& scene)
{
    // Generate COC textures
    frame.CommandBuffer->BeginMarker("Bokeh DOF");
    {
        ::Ref<RenderPassIO> cocTexture = PassManager::Get("COCTexture");
        ::Ref<RenderPassIO> depthTexture = PassManager::Get("GBufferDepth");

        float nearBegin = mFocalPlaneDistance - mFocusTransitionDistance;
		if (nearBegin < 0.0f)
			nearBegin = 0.0f;
		float nearEnd = mFocalPlaneDistance;
		float farBegin = mFocalPlaneDistance;
		float farEnd = mFocalPlaneDistance + mFocusTransitionDistance;

        struct Data {
            int PointClamp;
            int DepthBuffer;
            int Output;
            int Pad;

            float NearBegin;
            float NearEnd;
            float FarBegin;
            float FarEnd;

            glm::vec2 ProjParams;
            glm::vec2 Padding; 
        } data = {
            mPointClampSampler->BindlesssSampler(),
            depthTexture->ShaderResourceView->GetDescriptor().Index,
            cocTexture->UnorderedAccessView->GetDescriptor().Index,
            0,
            nearBegin,
            nearEnd,
            farBegin,
            farEnd,

            glm::vec2(scene.Camera.Projection()[2][2], scene.Camera.Projection()[2][3]),
            glm::vec2(0.0f)  
        };

        frame.CommandBuffer->BeginMarker("COC Generation");
        frame.CommandBuffer->SetComputePipeline(mCOCGeneration);
        frame.CommandBuffer->ComputePushConstants(&data, sizeof(data), 0);
        frame.CommandBuffer->Dispatch(glm::max(cocTexture->Desc.Width / 7, 1u), glm::max(cocTexture->Desc.Height / 7, 1u), 1);
        frame.CommandBuffer->EndMarker();
    }
    frame.CommandBuffer->EndMarker();
}

void BokehDOF::UI(const Frame& frame)
{
    if (ImGui::TreeNodeEx("Bokeh Depth of Field", ImGuiTreeNodeFlags_Framed)) {
        ImGui::SliderFloat("Focal Plane Distance", &mFocalPlaneDistance, CAMERA_NEAR, CAMERA_FAR);
        ImGui::SliderFloat("Focus Transition Distance", &mFocusTransitionDistance, CAMERA_NEAR, CAMERA_FAR);
        if (ImGui::TreeNodeEx("COC Texture", ImGuiTreeNodeFlags_Framed)) {
            ::Ref<RenderPassIO> cocTexture = PassManager::Get("COCTexture");
            frame.CommandBuffer->Barrier(cocTexture->Texture, ResourceLayout::Shader);
            ImGui::Image((ImTextureID)cocTexture->ShaderResourceView->GetDescriptor().GPU.ptr, ImVec2(256, 256));
            ImGui::TreePop();
        }
        ImGui::TreePop();
    }
}
