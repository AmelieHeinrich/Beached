//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-07 22:41:04
//

#include <Renderer/Techniques/CSM.hpp>
#include <Renderer/Techniques/Debug.hpp>

#include <Core/Logger.hpp>

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

    Asset::Handle vertexShader = AssetManager::Get("Assets/Shaders/Shadow/Vertex.hlsl", AssetType::Shader);
    Asset::Handle fragmentShader = AssetManager::Get("Assets/Shaders/Shadow/Fragment.hlsl", AssetType::Shader);

    GraphicsPipelineSpecs specs;
    specs.Bytecodes[ShaderType::Vertex] = vertexShader->Shader;
    specs.Bytecodes[ShaderType::Fragment] = fragmentShader->Shader;
    specs.Cull = CullMode::Back;
    specs.DepthEnabled = true;
    specs.Depth = DepthOperation::Less;
    specs.DepthFormat = TextureFormat::Depth32;
    specs.Signature = rhi->CreateRootSignature({ RootType::PushConstant }, sizeof(glm::mat4) * 3);
    
    mPipeline = rhi->CreateGraphicsPipeline(specs);
}

void CSM::Render(const Frame& frame, Scene& scene)
{
    // Generate frustums
    if (!mFreezeCascades) {
        UpdateCascades(scene);
    } else {
        for (int i = 0; i < SHADOW_CASCADE_COUNT; i++) {
            Debug::DrawFrustum(mCascades[i].View, mCascades[i].Proj, glm::vec3(0.3f, 0.5f, 0.8f));
        }
    }

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
                if (!Camera::IsBoxInFrustum(mCascades[i].Proj * mCascades[i].View, primitive.AABB, globalTransform))
                    continue;

                struct PushConstants {
                    glm::mat4 transform;
                    glm::mat4 view;
                    glm::mat4 proj;
                } Constants = {
                    globalTransform,
                    mCascades[i].View,
                    mCascades[i].Proj
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

    if (ImGui::TreeNodeEx("Cascaded Shadow Maps", ImGuiTreeNodeFlags_Framed)) {
        ImGui::SliderFloat("Far Plane Multiplier", &mZMult, 1.0f, 10.0f, "%.1f");
        ImGui::SliderFloat("Shadow Split Lambda", &mShadowSplitLambda, 0.0f, 1.0f, "%.2f");
        ImGui::Checkbox("Freeze Cascades", &mFreezeCascades);
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
    UInt32 cascadeSize = PassManager::Get("ShadowCascade0")->Desc.Width;
    Vector<float> splits(SHADOW_CASCADE_COUNT + 1);

    // Precompute cascade splits using logarithmic split
    splits[0] = CAMERA_NEAR;
    splits[SHADOW_CASCADE_COUNT] = CAMERA_FAR;
    for (int i = 1; i <= SHADOW_CASCADE_COUNT; ++i) {
        float linearSplit = CAMERA_NEAR + (CAMERA_FAR - CAMERA_NEAR) * (static_cast<float>(i) / SHADOW_CASCADE_COUNT);
        float logSplit = CAMERA_NEAR * std::pow(CAMERA_FAR / CAMERA_NEAR, static_cast<float>(i) / SHADOW_CASCADE_COUNT);

        // Blend the splits using the lambda parameter
        splits[i] = mShadowSplitLambda * logSplit + (1.0f - mShadowSplitLambda) * linearSplit;
    }

    for (int i = 0; i < SHADOW_CASCADE_COUNT; ++i) {
        // Get frustum corners for the cascade in view space
        Vector<glm::vec4> corners = scene.Camera.CornersForCascade(splits[i], splits[i + 1]);

        // Calculate center
        glm::vec3 center(0.0f);
        for (const glm::vec4& corner : corners) {
            center += glm::vec3(corner);
        }
        center /= corners.size();

        // Adjust light's up vector
        glm::vec3 up(0.0f, 1.0f, 0.0f);
        if (glm::abs(glm::dot(scene.Sun.Direction, up)) > 0.999f) {
            up = glm::vec3(1.0f, 0.0f, 0.0f);
        }

        // Calculate light-space bounding sphere
        glm::vec3 minBounds(FLT_MAX), maxBounds(-FLT_MAX);
        float sphereRadius = 0.0f;
        for (auto& corner : corners) {
            float dist = glm::length(glm::vec3(corner) - center);
            sphereRadius = std::max(sphereRadius, dist);
        }
        sphereRadius = std::ceil(sphereRadius * 16.0f) / 16.0f;
        maxBounds = glm::vec3(sphereRadius);
        minBounds = -maxBounds;

        // Get extents and create view matrix
        glm::vec3 cascadeExtents = maxBounds - minBounds;
        glm::vec3 shadowCameraPos = center - scene.Sun.Direction * -minBounds.z;

        glm::mat4 lightView = glm::lookAt(shadowCameraPos, center, up);
        glm::mat4 lightProjection = glm::ortho(
            minBounds.x,
            maxBounds.x,
            minBounds.y,
            maxBounds.y,
            minBounds.z,
            maxBounds.z * mZMult
        );

        // Texel snap
        {
            glm::mat4 shadowMatrix = lightProjection * lightView;
            glm::vec4 shadowOrigin = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
            shadowOrigin = shadowMatrix * shadowOrigin;
            shadowOrigin = glm::scale(glm::mat4(1.0f), glm::vec3(cascadeSize / 2)) * shadowOrigin;
    
            glm::vec4 roundedOrigin = glm::round(shadowOrigin);
            glm::vec4 roundOffset = roundedOrigin - shadowOrigin;
            roundOffset = roundOffset * (2.0f / cascadeSize);
            roundOffset.z = 0.0f;
            roundOffset.w = 0.0f;
            lightProjection[3] += roundOffset;
        }

        // Store results
        mCascades[i].Split = splits[i + 1];
        mCascades[i].View = lightView;
        mCascades[i].Proj = lightProjection;
    }
}
