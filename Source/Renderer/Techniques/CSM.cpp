//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-07 22:41:04
//

#include <Renderer/Techniques/CSM.hpp>
#include <Renderer/Techniques/Debug.hpp>

#include <imgui.h>

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
    if (!mFreezeFrustum) {
        mFrozenView = scene.Camera.View();
        mFrozenProj = scene.Camera.Projection();

        mLightMatrices.clear();
        std::array<float, SHADOW_CASCADE_COUNT + 1> cascadeLevels = ComputeCascadeLevels(CAMERA_NEAR, CAMERA_FAR);
        for (int i = 0; i < SHADOW_CASCADE_COUNT; ++i) {
            float cascadeStart = cascadeLevels[i];
            float cascadeEnd = cascadeLevels[i + 1];
            mLightMatrices.push_back(GetLightSpaceMatrix(frame, scene, cascadeStart, cascadeEnd));
        }
    } else {
        Debug::DrawFrustum(mFrozenProj * mFrozenView, glm::vec3(1.0f, 0.0f, 0.0f));
        for (LightMatrix matrix : mLightMatrices) {
            Debug::DrawFrustum(matrix.Projection * matrix.View, glm::vec3(0.0f, 1.0f, 0.0f));
        }
    }

    // Draw
    Vector<::Ref<RenderPassIO>> cascades = {
        PassManager::Get("ShadowCascade0"),
        PassManager::Get("ShadowCascade1"),
        PassManager::Get("ShadowCascade2"),
        PassManager::Get("ShadowCascade3"),
    };

    frame.CommandBuffer->BeginMarker("Cascaded Shadow Maps");
    for (int i = 0; i < SHADOW_CASCADE_COUNT; i++) {
        frame.CommandBuffer->BeginMarker("Cascade " + std::to_string(i));
        frame.CommandBuffer->Barrier(cascades[i]->Texture, ResourceLayout::DepthWrite);
        frame.CommandBuffer->SetGraphicsPipeline(mPipeline);
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
                struct PushConstants {
                    glm::mat4 transform;
                    glm::mat4 view;
                    glm::mat4 projection;
                } Constants = {
                    globalTransform,
                    mLightMatrices[i].View,
                    mLightMatrices[i].Projection
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
        ImGui::Checkbox("Freeze Frustum", &mFreezeFrustum);
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

Array<float, SHADOW_CASCADE_COUNT + 1> CSM::ComputeCascadeLevels(float cameraNear, float cameraFar)
{
    Array<float, SHADOW_CASCADE_COUNT + 1> levels;
    levels[0] = cameraNear;

    // Compute cascade levels using a logarithmic distribution
    for (int i = 1; i <= SHADOW_CASCADE_COUNT; ++i) {
        float fraction = static_cast<float>(i) / SHADOW_CASCADE_COUNT;
        levels[i] = cameraNear * pow(cameraFar / cameraNear, fraction);  // Non-linear spacing
    }

    return levels;
}

CSM::LightMatrix CSM::GetLightSpaceMatrix(const Frame& frame, const Scene& scene, float near, float far)
{
    // Step 1: Compute perspective projection for the camera frustum
    glm::mat4 projection = glm::perspective(glm::radians(90.0f), (float)frame.Width / (float)frame.Height, near, far);

    // Step 2: Get frustum corners (this might need to be adjusted)
    Vector<glm::vec4> corners = Camera::FrustumCorners(scene.Camera.View(), projection);

    // Step 3: Normalize the light direction
    glm::vec3 lightDir = glm::normalize(-scene.Sun.Direction);
    assert(glm::length(lightDir) > 0.0f && "Sun direction vector must not be zero");

    // Step 4: Compute the frustum center
    glm::vec3 center = glm::vec3(0.0f);
    for (const glm::vec4& corner : corners) {
        center += glm::vec3(corner);
    }
    center /= corners.size();

    // Step 5: Construct the light's view matrix (account for left-handed coordinate system)
    glm::vec3 up = glm::abs(lightDir.y) > 0.99f ? glm::vec3(1.0f, 0.0f, 0.0f) : glm::vec3(0.0f, 1.0f, 0.0f);
    glm::mat4 lightView = glm::lookAt(center + lightDir, center, up);

    // Step 6: Transform frustum corners into light view space
    float minX = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::lowest();
    float minY = std::numeric_limits<float>::max();
    float maxY = std::numeric_limits<float>::lowest();
    float minZ = std::numeric_limits<float>::max();
    float maxZ = std::numeric_limits<float>::lowest();

    for (const glm::vec4& corner : corners) {
        glm::vec4 trf = lightView * corner; // Transform into light view space
        minX = std::min(minX, trf.x);
        maxX = std::max(maxX, trf.x);
        minY = std::min(minY, trf.y);
        maxY = std::max(maxY, trf.y);
        minZ = std::min(minZ, trf.z);
        maxZ = std::max(maxZ, trf.z);
    }

    // Step 8: Construct the orthographic projection matrix for DirectX (Zero-to-One depth)
    glm::mat4 lightProjection = glm::ortho(minX, maxX, minY, maxY, minZ, maxZ);

    // Step 9: Return combined view and projection matrices
    return { false, lightView, lightProjection };
}
