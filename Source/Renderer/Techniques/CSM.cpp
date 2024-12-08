//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-07 22:41:04
//

#include <Renderer/Techniques/CSM.hpp>
#include <Renderer/Techniques/Debug.hpp>

#include <imgui.h>

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
    specs.Signature = rhi->CreateRootSignature({ RootType::PushConstant }, sizeof(glm::mat4) * 2);
    
    mPipeline = rhi->CreateGraphicsPipeline(specs);
}

void CSM::Render(const Frame& frame, const Scene& scene)
{
    // Generate frustums
    if (!mFreezeFrustum) {
        mFrozenView = scene.Camera.View();
        mFrozenProj = scene.Camera.Projection();

        mLightMatrices.clear();
        for (int i = 0; i < SHADOW_CASCADE_COUNT + 1; i++) {
            if (i == 0) {
                mLightMatrices.push_back(GetLightSpaceMatrix(scene, CAMERA_NEAR, SHADOW_CASCADE_LEVELS[i]));
            } else if (i < SHADOW_CASCADE_COUNT) {
                mLightMatrices.push_back(GetLightSpaceMatrix(scene, SHADOW_CASCADE_LEVELS[i - 1], SHADOW_CASCADE_LEVELS[i]));
            } else {
                mLightMatrices.push_back(GetLightSpaceMatrix(scene, SHADOW_CASCADE_LEVELS[i - 1], CAMERA_FAR));
            }
        }
    } else {
        Debug::DrawFrustum(mFrozenProj * mFrozenView);
        for (glm::mat4 matrix : mLightMatrices) {
            Debug::DrawFrustum(matrix, glm::vec3(1.0f, 0.0f, 0.0f));
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
                    glm::mat4 light;
                } Constants = {
                    globalTransform,
                    mLightMatrices[i]
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

void CSM::UI()
{
    if (ImGui::TreeNodeEx("Cascaded Shadow Maps", ImGuiTreeNodeFlags_Framed)) {
        ImGui::Checkbox("Freeze Frustum", &mFreezeFrustum);
        ImGui::TreePop();
    }
}

glm::mat4 CSM::GetLightSpaceMatrix(const Scene& scene, float near, float far)
{
    // View
    Vector<glm::vec4> corners = scene.Camera.Corners();
    glm::vec3 center = glm::vec3(0.0f);
    for (glm::vec4& corner : corners) {
        center += glm::vec3(corner);
    }
    center /= corners.size();
    glm::mat4 lightView = glm::lookAt(center + scene.Sun.Direction, center, glm::vec3(0.0f, 1.0f, 0.0f));
    
    // Projection
    float minX = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::lowest();
    float minY = std::numeric_limits<float>::max();
    float maxY = std::numeric_limits<float>::lowest();
    float minZ = std::numeric_limits<float>::max();
    float maxZ = std::numeric_limits<float>::lowest();
    for (const auto& v : corners) {
        const auto trf = lightView * v;
        minX = std::min(minX, trf.x);
        maxX = std::max(maxX, trf.x);
        minY = std::min(minY, trf.y);
        maxY = std::max(maxY, trf.y);
        minZ = std::min(minZ, trf.z);
        maxZ = std::max(maxZ, trf.z);
    }

    constexpr float zMult = 10.0f;
    if (minZ < 0) {
        minZ *= zMult;
    } else {
        minZ /= zMult;
    }
    if (maxZ < 0) {
        maxZ /= zMult;
    } else {
        maxZ *= zMult;
    }
    glm::mat4 lightProjection = glm::ortho(minX, maxX, minY, maxY, minZ, maxZ);
    return lightProjection * lightView;
}
