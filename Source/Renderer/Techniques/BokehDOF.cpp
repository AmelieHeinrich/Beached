//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2025-01-11 06:46:05
//

#include <Renderer/Techniques/BokehDOF.hpp>

#include <imgui.h>
#include <imgui_internal.h>
#include <glm/gtc/type_ptr.hpp>

BokehDOF::BokehDOF(RHI::Ref rhi)
    : RenderPass(rhi)
{
    // COC Compute
    {
        Asset::Handle cocShader = AssetManager::Get("Assets/Shaders/BokehDOF/COCCompute.hlsl", AssetType::Shader);

        auto cocSignature = mRHI->CreateRootSignature({ RootType::PushConstant }, sizeof(glm::vec4) * 3);
        mCOCGeneration = mRHI->CreateComputePipeline(cocShader->Shader, cocSignature);
    }
    // Downsample
    {
        Asset::Handle shader = AssetManager::Get("Assets/Shaders/BokehDOF/DownsampleCompute.hlsl", AssetType::Shader);

        auto signature = mRHI->CreateRootSignature({ RootType::PushConstant }, sizeof(glm::vec4) * 2);
        mDownsample = mRHI->CreateComputePipeline(shader->Shader, signature);   
    }
    // Max
    {
        Asset::Handle shader = AssetManager::Get("Assets/Shaders/BokehDOF/MaxCompute.hlsl", AssetType::Shader);

        auto signature = mRHI->CreateRootSignature({ RootType::PushConstant }, sizeof(glm::vec4));
        mMaxFilter = mRHI->CreateComputePipeline(shader->Shader, signature);  
    }
    // Blur
    {
        Asset::Handle shader = AssetManager::Get("Assets/Shaders/BokehDOF/BlurCompute.hlsl", AssetType::Shader);

        auto signature = mRHI->CreateRootSignature({ RootType::PushConstant }, sizeof(glm::vec4));
        mBlurFilter = mRHI->CreateComputePipeline(shader->Shader, signature);  
    }
    // Computation
    {
        Asset::Handle shader = AssetManager::Get("Assets/Shaders/BokehDOF/BokehCompute.hlsl", AssetType::Shader);

        auto signature = mRHI->CreateRootSignature({ RootType::PushConstant }, sizeof(glm::vec4) * 3);
        mBokehFilter = mRHI->CreateComputePipeline(shader->Shader, signature);
    }
    // Composite
    {
        Asset::Handle shader = AssetManager::Get("Assets/Shaders/BokehDOF/CompositeCompute.hlsl", AssetType::Shader);

        auto signature = mRHI->CreateRootSignature({ RootType::PushConstant }, sizeof(glm::vec4) * 3);
        mComposite = mRHI->CreateComputePipeline(shader->Shader, signature);
    }

    // Samplers
    mPointClampSampler = mRHI->CreateSampler(SamplerAddress::Clamp, SamplerFilter::Nearest);
    mLinearClampSampler = mRHI->CreateSampler(SamplerAddress::Clamp, SamplerFilter::Linear);
}

void BokehDOF::Render(const Frame& frame, Scene& scene)
{
    if (!mEnable)
        return;

    frame.CommandBuffer->BeginMarker("Bokeh DOF");
    // Generate COC textures
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
        frame.CommandBuffer->Barrier(depthTexture->Texture, ResourceLayout::Shader);
        frame.CommandBuffer->Barrier(cocTexture->Texture, ResourceLayout::Storage);
        frame.CommandBuffer->SetComputePipeline(mCOCGeneration);
        frame.CommandBuffer->ComputePushConstants(&data, sizeof(data), 0);
        frame.CommandBuffer->Dispatch(glm::max(cocTexture->Desc.Width / 7, 1u), glm::max(cocTexture->Desc.Height / 7, 1u), 1);
        frame.CommandBuffer->UAVBarrier(cocTexture->Texture);
        frame.CommandBuffer->EndMarker();
    }
    // Downsample
    {
        ::Ref<RenderPassIO> colorTexture = PassManager::Get("MainColorBuffer");
        ::Ref<RenderPassIO> cocTexture = PassManager::Get("COCTexture");
        ::Ref<RenderPassIO> colorX4 = PassManager::Get("DOFColorX4");
        ::Ref<RenderPassIO> cocMulFarX4 = PassManager::Get("DOFMulFarColorX4");
        ::Ref<RenderPassIO> cocX4 = PassManager::Get("COCTextureX4");

        struct Data {
            int PointClamp;
            int LinearClamp;
            int ColorTexture;
            int COCTexture;

            int ColorX4;
            int COCMulFarX4;
            int COCX4;
            int Pad;
        } data = {
            mPointClampSampler->BindlesssSampler(),
            mLinearClampSampler->BindlesssSampler(),
            colorTexture->ShaderResourceView->GetDescriptor().Index,
            cocTexture->ShaderResourceView->GetDescriptor().Index,
            
            colorX4->UnorderedAccessView->GetDescriptor().Index,
            cocMulFarX4->UnorderedAccessView->GetDescriptor().Index,
            cocX4->UnorderedAccessView->GetDescriptor().Index,
            0
        };

        frame.CommandBuffer->BeginMarker("Downsample Targets");
        frame.CommandBuffer->Barrier(colorTexture->Texture, ResourceLayout::Shader);
        frame.CommandBuffer->Barrier(cocTexture->Texture, ResourceLayout::Shader);
        frame.CommandBuffer->Barrier(colorX4->Texture, ResourceLayout::Storage);
        frame.CommandBuffer->Barrier(cocMulFarX4->Texture, ResourceLayout::Storage);
        frame.CommandBuffer->Barrier(cocX4->Texture, ResourceLayout::Storage);
        frame.CommandBuffer->SetComputePipeline(mDownsample);
        frame.CommandBuffer->ComputePushConstants(&data, sizeof(data), 0);
        frame.CommandBuffer->Dispatch(glm::max(cocX4->Desc.Width / 7, 1u), glm::max(cocX4->Desc.Height / 7, 1u), 1);
        frame.CommandBuffer->UAVBarrier(colorX4->Texture);
        frame.CommandBuffer->UAVBarrier(cocMulFarX4->Texture);
        frame.CommandBuffer->UAVBarrier(cocX4->Texture);
        frame.CommandBuffer->EndMarker();
    }
    // Max filter
    {
        ::Ref<RenderPassIO> cocX4 = PassManager::Get("COCTextureX4");
        ::Ref<RenderPassIO> nearBlurX4 = PassManager::Get("DOFNearBlurX4");
    
        struct Data {
            int Input;
            int Output;
            glm::ivec2 Pad;
        } data = {
            cocX4->UnorderedAccessView->GetDescriptor().Index,
            nearBlurX4->UnorderedAccessView->GetDescriptor().Index,
            glm::ivec2(0)
        };

        frame.CommandBuffer->BeginMarker("Max Near Field");
        frame.CommandBuffer->Barrier(cocX4->Texture, ResourceLayout::Storage);
        frame.CommandBuffer->Barrier(nearBlurX4->Texture, ResourceLayout::Storage);
        frame.CommandBuffer->SetComputePipeline(mMaxFilter);
        frame.CommandBuffer->ComputePushConstants(&data, sizeof(data), 0);
        frame.CommandBuffer->Dispatch(glm::max(cocX4->Desc.Width / 7, 1u), glm::max(cocX4->Desc.Height / 7, 1u), 1);
        frame.CommandBuffer->UAVBarrier(cocX4->Texture);
        frame.CommandBuffer->UAVBarrier(nearBlurX4->Texture);
        frame.CommandBuffer->EndMarker();
    }
    // Blur filter
    {
        ::Ref<RenderPassIO> nearX4 = PassManager::Get("DOFNearBlurX4");
    
        struct Data {
            int Output;
            glm::ivec3 Pad;
        } data = {
            nearX4->UnorderedAccessView->GetDescriptor().Index,
            glm::ivec3(0)
        };

        frame.CommandBuffer->BeginMarker("Blur Near Field");
        frame.CommandBuffer->Barrier(nearX4->Texture, ResourceLayout::Storage);
        frame.CommandBuffer->SetComputePipeline(mBlurFilter);
        frame.CommandBuffer->ComputePushConstants(&data, sizeof(data), 0);
        frame.CommandBuffer->Dispatch(glm::max(nearX4->Desc.Width / 7, 1u), glm::max(nearX4->Desc.Height / 7, 1u), 1);
        frame.CommandBuffer->UAVBarrier(nearX4->Texture);
        frame.CommandBuffer->EndMarker();
    }
    // Bokeh filter
    {
        ::Ref<RenderPassIO> dofNear = PassManager::Get("DOFNearX4");
        ::Ref<RenderPassIO> dofFar = PassManager::Get("DOFFarX4");
        ::Ref<RenderPassIO> cocX4 = PassManager::Get("COCTextureX4");
        ::Ref<RenderPassIO> nearX4 = PassManager::Get("DOFNearBlurX4");
        ::Ref<RenderPassIO> colorX4 = PassManager::Get("DOFColorX4");
        ::Ref<RenderPassIO> cocMulFarX4 = PassManager::Get("DOFMulFarColorX4");

        struct Data {
            int LinearSampler;
            int PointSampler;
            int DOFNear;
            int DOFFar;

            int COCTexture;
            int COCNearTexture;
            int ColorTexture;
            int ColorMulFar;

            float KernelSize;
            glm::vec3 Pad;
        } data = {
            mLinearClampSampler->BindlesssSampler(),
            mPointClampSampler->BindlesssSampler(),
            dofNear->UnorderedAccessView->GetDescriptor().Index,
            dofFar->UnorderedAccessView->GetDescriptor().Index,
            cocX4->ShaderResourceView->GetDescriptor().Index,
            nearX4->ShaderResourceView->GetDescriptor().Index,
            colorX4->ShaderResourceView->GetDescriptor().Index,
            cocMulFarX4->ShaderResourceView->GetDescriptor().Index,
            1.0f,
            glm::vec3(0.0f)
        };

        frame.CommandBuffer->BeginMarker("Bokeh Computation");
        frame.CommandBuffer->Barrier(dofNear->Texture, ResourceLayout::Storage);
        frame.CommandBuffer->Barrier(dofFar->Texture, ResourceLayout::Storage);
        frame.CommandBuffer->Barrier(cocX4->Texture, ResourceLayout::Shader);
        frame.CommandBuffer->Barrier(nearX4->Texture, ResourceLayout::Shader);
        frame.CommandBuffer->Barrier(colorX4->Texture, ResourceLayout::Shader);
        frame.CommandBuffer->Barrier(cocMulFarX4->Texture, ResourceLayout::Shader);
        frame.CommandBuffer->SetComputePipeline(mBokehFilter);
        frame.CommandBuffer->ComputePushConstants(&data, sizeof(data), 0);
        frame.CommandBuffer->Dispatch(glm::max(dofNear->Desc.Width / 7, 1u), glm::max(dofNear->Desc.Height / 7, 1u), 1);
        frame.CommandBuffer->UAVBarrier(dofNear->Texture);
        frame.CommandBuffer->UAVBarrier(dofFar->Texture);
        frame.CommandBuffer->EndMarker();
    }
    // Composite
    {
        ::Ref<RenderPassIO> colorTexture = PassManager::Get("MainColorBuffer");
        ::Ref<RenderPassIO> cocTexture = PassManager::Get("COCTexture");
        ::Ref<RenderPassIO> cocTextureX4 = PassManager::Get("COCTextureX4");
        ::Ref<RenderPassIO> cocNearTexture = PassManager::Get("DOFNearBlurX4");
        ::Ref<RenderPassIO> nearTexture = PassManager::Get("DOFNearX4");
        ::Ref<RenderPassIO> farTexture = PassManager::Get("DOFFarX4");

        struct Data
        {
            int PointSampler;
            int LinearSampler;
            int ColorTexture;
            int COCTexture;

            int COCTextureX4;
            int COCNear;
            int DOFNear;
            int DOFFar;

            float Strength;
            glm::vec3 Pad;
        } data = {
            mLinearClampSampler->BindlesssSampler(),
            mPointClampSampler->BindlesssSampler(),
            colorTexture->UnorderedAccessView->GetDescriptor().Index,
            cocTexture->ShaderResourceView->GetDescriptor().Index,
            cocTextureX4->ShaderResourceView->GetDescriptor().Index,
            cocNearTexture->ShaderResourceView->GetDescriptor().Index,
            nearTexture->ShaderResourceView->GetDescriptor().Index,
            farTexture->ShaderResourceView->GetDescriptor().Index,
            1.0f,
            glm::vec3(0.0f)
        };

        frame.CommandBuffer->BeginMarker("Composite");
        frame.CommandBuffer->Barrier(colorTexture->Texture, ResourceLayout::Storage);
        frame.CommandBuffer->Barrier(cocTexture->Texture, ResourceLayout::Shader);
        frame.CommandBuffer->Barrier(cocTextureX4->Texture, ResourceLayout::Shader);
        frame.CommandBuffer->Barrier(cocNearTexture->Texture, ResourceLayout::Shader);
        frame.CommandBuffer->Barrier(nearTexture->Texture, ResourceLayout::Shader);
        frame.CommandBuffer->Barrier(farTexture->Texture, ResourceLayout::Shader);
        frame.CommandBuffer->SetComputePipeline(mComposite);
        frame.CommandBuffer->ComputePushConstants(&data, sizeof(data), 0);
        frame.CommandBuffer->Dispatch(glm::max(colorTexture->Desc.Width / 7, 1u), glm::max(colorTexture->Desc.Height / 7, 1u), 1);
        frame.CommandBuffer->UAVBarrier(colorTexture->Texture);
        frame.CommandBuffer->EndMarker();
    }
    frame.CommandBuffer->EndMarker();
}

void BokehDOF::UI(const Frame& frame)
{
    if (ImGui::TreeNodeEx("Bokeh Depth of Field", ImGuiTreeNodeFlags_Framed)) {
        ImGui::Checkbox("Enable", &mEnable);
        if (!mEnable) {
            ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
        }
        ImGui::SliderFloat("Focal Plane Distance", &mFocalPlaneDistance, CAMERA_NEAR, CAMERA_FAR);
        ImGui::SliderFloat("Focus Transition Distance", &mFocusTransitionDistance, CAMERA_NEAR, CAMERA_FAR);
        if (ImGui::TreeNodeEx("COC Texture", ImGuiTreeNodeFlags_Framed)) {
            ::Ref<RenderPassIO> cocTexture = PassManager::Get("COCTexture");
            frame.CommandBuffer->Barrier(cocTexture->Texture, ResourceLayout::Shader);
            ImGui::Image((ImTextureID)cocTexture->ShaderResourceView->GetDescriptor().GPU.ptr, ImVec2(256, 256));
            ImGui::TreePop();
        }
        if (!mEnable) {
            ImGui::PopItemFlag();
            ImGui::PopStyleVar();
        }
        ImGui::TreePop();
    }
}
