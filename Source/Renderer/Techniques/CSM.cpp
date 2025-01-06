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

void CSM::Render(const Frame& frame, const Scene& scene)
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
                // Cull objects that are not seen
                glm::mat4 widePerspective = glm::perspective(glm::radians(180.0f), (float)frame.Width / (float)frame.Height, CAMERA_NEAR, CAMERA_FAR);
                if (!Camera::IsBoxInFrustum(widePerspective * scene.Camera.View(), primitive.AABB, globalTransform) && Settings::Get().FrustumCull)
                    continue;
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
    Vector<float> splits(SHADOW_CASCADE_COUNT + 1);

    // Precompute cascade splits using logarithmic split
    splits[0] = CAMERA_NEAR;
    for (int i = 1; i <= SHADOW_CASCADE_COUNT; ++i) {
        // Uniform split
        float uniformSplit = CAMERA_NEAR + (CAMERA_FAR - CAMERA_NEAR) * (float(i) / SHADOW_CASCADE_COUNT);

        // Logarithmic split
        float logSplit = CAMERA_NEAR * std::pow(CAMERA_FAR / CAMERA_NEAR, float(i) / SHADOW_CASCADE_COUNT);

        // Blend using lambda
        splits[i] = SHADOW_SPLIT_LAMBDA * logSplit + (1.0f - SHADOW_SPLIT_LAMBDA) * uniformSplit;
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

        // Light's view matrix
        glm::mat4 lightView = glm::lookAt(center - scene.Sun.Direction, center, up);

        // Calculate light-space bounding box
        glm::vec3 minBounds(FLT_MAX), maxBounds(-FLT_MAX);
        for (glm::vec4 corner : corners) {
            glm::vec4 lightSpaceCorner = lightView * corner;
            minBounds = glm::min(minBounds, glm::vec3(lightSpaceCorner));
            maxBounds = glm::max(maxBounds, glm::vec3(lightSpaceCorner));
        }
        
        float min = 0.0f;
        min = glm::min(min, minBounds.x);
        min = glm::min(min, minBounds.y);

        float max = 0.0f;
        max = glm::max(max, maxBounds.x);
        max = glm::max(max, maxBounds.y);

        // Projection matrix
        glm::mat4 lightProjection = glm::ortho(
            min,  // Left
            max,  // Right
            min,  // Bottom
            max,  // Top
            minBounds.z,  // Near
            maxBounds.z   // Far
        );

        // Store results
        mCascades[i].Split = splits[i + 1];
        mCascades[i].View = lightView;
        mCascades[i].Proj = lightProjection;
    }
}
