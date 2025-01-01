//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-07 22:41:04
//

#include <Renderer/Techniques/CSM.hpp>
#include <Renderer/Techniques/Debug.hpp>

#include <imgui.h>
#include <Settings.hpp>

#undef near
#undef far

CSM::CSM(RHI::Ref rhi)
    : RenderPass(rhi)
{
    Vector<::Ref<RenderPassIO>> cascades = {
        PassManager::Get("ShadowCascade0"),
        PassManager::Get("ShadowCascade1"),
        PassManager::Get("ShadowCascade2"),
        PassManager::Get("ShadowCascade3"),
    };
    for (::Ref<RenderPassIO> cascade : cascades) {
        cascade->ShaderResourceView = mRHI->CreateView(cascade->Texture, ViewType::ShaderResource, ViewDimension::Texture, TextureFormat::R32Float);
    }

    Asset::Handle vertexShader = AssetManager::Get("Assets/Shaders/CSM/Vertex.hlsl", AssetType::Shader);
    Asset::Handle fragmentShader = AssetManager::Get("Assets/Shaders/CSM/Fragment.hlsl", AssetType::Shader);

    GraphicsPipelineSpecs specs;
    specs.Bytecodes[ShaderType::Vertex] = vertexShader->Shader;
    specs.Bytecodes[ShaderType::Fragment] = fragmentShader->Shader;
    specs.Cull = CullMode::Front;
    specs.DepthEnabled = true;
    specs.Depth = DepthOperation::Less;
    specs.DepthFormat = TextureFormat::Depth32;
    specs.Signature = rhi->CreateRootSignature({ RootType::PushConstant }, sizeof(glm::mat4) * 3);
    
    mPipeline = rhi->CreateGraphicsPipeline(specs);
}

void CSM::Render(const Frame& frame, const Scene& scene)
{
    // Generate frustums
    UpdateCascades(scene);

    // Draw
    Vector<::Ref<RenderPassIO>> cascades = {
        PassManager::Get("ShadowCascade0"),
        PassManager::Get("ShadowCascade1"),
        PassManager::Get("ShadowCascade2"),
        PassManager::Get("ShadowCascade3"),
    };

    // Update frustum buffer
    {
        ::Ref<RenderPassIO> cascadeRingBuffer = PassManager::Get("CascadeRingBuffer");
        for (int i = 0; i < SHADOW_CASCADE_COUNT; i++) {
            mCascades[i].SRVIndex = cascades[i]->ShaderResourceView->GetDescriptor().Index;
        }

        cascadeRingBuffer->RingBuffer[frame.FrameIndex]->CopyMapped(mCascades.data(), sizeof(Cascade) * SHADOW_CASCADE_COUNT);
    }

    frame.CommandBuffer->BeginMarker("Cascaded Shadow Maps");
    frame.CommandBuffer->SetGraphicsPipeline(mPipeline);
    for (int i = 0; i < SHADOW_CASCADE_COUNT; i++) {
        frame.CommandBuffer->BeginMarker("Cascade " + std::to_string(i));
        frame.CommandBuffer->Barrier(cascades[i]->Texture, ResourceLayout::DepthWrite);
        frame.CommandBuffer->SetRenderTargets({}, cascades[i]->DepthTargetView);
        frame.CommandBuffer->ClearDepth(cascades[i]->DepthTargetView);
        frame.CommandBuffer->SetViewport(0, 0, cascades[i]->Desc.Width, cascades[i]->Desc.Height);
        frame.CommandBuffer->SetTopology(Topology::TriangleList);
        std::function<void(Frame frame, GLTFNode*, GLTF* model, glm::mat4 transform)> drawNode = [&](Frame frame, GLTFNode* node, GLTF* model, glm::mat4 transform) {
            if (!node) {
                return;
            }

            glm::mat4 globalTransform = transform * node->Transform;
            for (GLTFPrimitive primitive : node->Primitives) {
                if (!Camera::IsBoxInFrustum(mCascades[i].ViewProj, primitive.AABB, globalTransform) && Settings::Get().FrustumCull)
                    continue;

                struct PushConstants {
                    glm::mat4 transform;
                    glm::mat4 viewProjection;
                } Constants = {
                    globalTransform,
                    mCascades[i].ViewProj,
                };
                frame.CommandBuffer->GraphicsPushConstants(&Constants, sizeof(Constants), 0);
                frame.CommandBuffer->SetVertexBuffer(primitive.VertexBuffer);
                frame.CommandBuffer->SetIndexBuffer(primitive.IndexBuffer);
                frame.CommandBuffer->DrawIndexed(primitive.IndexCount);
            }

            if (!node->Children.empty()) {
                for (GLTFNode* child : node->Children) {
                    drawNode(frame, child, model, globalTransform);
                }
            }
        };
        for (auto& model : scene.Models) {
            drawNode(frame, model->Model.Root, &model->Model, glm::mat4(1.0f));
        }

        frame.CommandBuffer->Barrier(cascades[i]->Texture, ResourceLayout::Shader);
        frame.CommandBuffer->EndMarker();
    }
    frame.CommandBuffer->EndMarker();
}

void CSM::UI(const Frame& frame)
{
    Vector<::Ref<RenderPassIO>> cascades = {
        PassManager::Get("ShadowCascade0"),
        PassManager::Get("ShadowCascade1"),
        PassManager::Get("ShadowCascade2"),
        PassManager::Get("ShadowCascade3"),
    };

    if (ImGui::TreeNodeEx("Cascaded Shadow Maps", ImGuiTreeNodeFlags_Framed)) {;
        for (int i = 0; i < SHADOW_CASCADE_COUNT; i++) {
            if (ImGui::TreeNodeEx(("Cascade " + std::to_string(i)).data(), ImGuiTreeNodeFlags_Framed)) {
                frame.CommandBuffer->Barrier(cascades[i]->Texture, ResourceLayout::Shader);
                ImGui::Image((ImTextureID)cascades[i]->ShaderResourceView->GetDescriptor().GPU.ptr, ImVec2(128, 128));
                ImGui::TreePop();
            }
        }
        ImGui::TreePop();
    }
}

void CSM::UpdateCascades(const Scene& scene)
{
    float splits[SHADOW_CASCADE_COUNT];

    float nearClip = CAMERA_NEAR;
    float farClip = CAMERA_FAR;
    float clipRange = farClip - nearClip;

    float minZ = nearClip;
    float maxZ = nearClip + clipRange;

    float range = maxZ - minZ;
    float ratio = maxZ / minZ;

    // Calculate split depths based on view camera frustum
    for (int i = 0; i < SHADOW_CASCADE_COUNT; i++) {
        float p = (i + 1) / static_cast<float>(SHADOW_CASCADE_COUNT);
        float log = minZ + range * p;
        float uniform = minZ + range * p;
        float d = CASCADE_SPLIT_LAMBDA * (log - uniform) + uniform;
        mSplits[i] = (d - nearClip) / clipRange;
    }

	// Based on method presented in https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch10.html
    // Calculate orthographic projection matrix for each cascade
    float lastSplitDist = 0.0f;
    for (int i = 0; i < SHADOW_CASCADE_COUNT; i++) {
        float splitDist = mSplits[i];

        mCascades[i].Split = (CAMERA_NEAR + splitDist * clipRange);
        mCascades[i].ViewProj = glm::mat4(0.0f);

        lastSplitDist = mSplits[i];
    }
}
