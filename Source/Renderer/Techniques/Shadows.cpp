//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2025-01-07 12:43:59
//

#include <Renderer/Techniques/Shadows.hpp>
#include <Core/Logger.hpp>
#include <Renderer/Techniques/Debug.hpp>

#include <imgui.h>

Shadows::Shadows(RHI::Ref rhi)
    : RenderPass(rhi)
{
    {
        Asset::Handle vertexShader = AssetManager::Get("Assets/Shaders/Shadow/Vertex.hlsl",     AssetType::Shader);
        Asset::Handle fragmentShader = AssetManager::Get("Assets/Shaders/Shadow/Fragment.hlsl",     AssetType::Shader);

        GraphicsPipelineSpecs specs;
        specs.Bytecodes[ShaderType::Vertex] = vertexShader->Shader;
        specs.Bytecodes[ShaderType::Fragment] = fragmentShader->Shader;
        specs.Cull = CullMode::Back;
        specs.DepthEnabled = true;
        specs.DepthClampEnable = true;
        specs.Depth = DepthOperation::Less;
        specs.DepthFormat = TextureFormat::Depth32;
        specs.Signature = rhi->CreateRootSignature({ RootType::PushConstant }, sizeof(glm::mat4) * 3);

        mCascadePipeline = rhi->CreateGraphicsPipeline(specs);
    }
    {
        Asset::Handle vertexShader = AssetManager::Get("Assets/Shaders/PointShadow/Vertex.hlsl", AssetType::Shader);
        Asset::Handle fragmentShader = AssetManager::Get("Assets/Shaders/PointShadow/Fragment.hlsl", AssetType::Shader);

        GraphicsPipelineSpecs specs;
        specs.Bytecodes[ShaderType::Vertex] = vertexShader->Shader;
        specs.Bytecodes[ShaderType::Fragment] = fragmentShader->Shader;
        specs.Cull = CullMode::Back;
        specs.DepthEnabled = true;
        specs.Depth = DepthOperation::Less;
        specs.DepthFormat = TextureFormat::Depth32;
        specs.Signature = rhi->CreateRootSignature({ RootType::PushConstant }, sizeof(glm::mat4) * 3 + sizeof(glm::vec4));

        mPointPipeline = rhi->CreateGraphicsPipeline(specs);
    }
    {
        Asset::Handle vertexShader = AssetManager::Get("Assets/Shaders/SpotShadow/Vertex.hlsl", AssetType::Shader);
        Asset::Handle fragmentShader = AssetManager::Get("Assets/Shaders/SpotShadow/Fragment.hlsl", AssetType::Shader);

        GraphicsPipelineSpecs specs;
        specs.Bytecodes[ShaderType::Vertex] = vertexShader->Shader;
        specs.Bytecodes[ShaderType::Fragment] = fragmentShader->Shader;
        specs.Cull = CullMode::Front;
        specs.DepthEnabled = true;
        specs.Depth = DepthOperation::Less;
        specs.DepthFormat = TextureFormat::Depth32;
        specs.Signature = rhi->CreateRootSignature({ RootType::PushConstant }, sizeof(glm::mat4) * 3);

        mSpotPipeline = rhi->CreateGraphicsPipeline(specs);
    }
}

void Shadows::Bake(Scene& scene)
{
    for (int i = 0; i < scene.PointLights.size(); i++) {
        if (scene.PointLights[i].CastShadows) {
            PointLightShadow shadow;
            shadow.Parent = &scene.PointLights[i];

            TextureDesc desc;
            desc.Name = "Point Light Shadow Map";
            desc.Usage = TextureUsage::DepthTarget;
            desc.Width = POINT_LIGHT_SHADOW_DIMENSION;
            desc.Height = POINT_LIGHT_SHADOW_DIMENSION;
            desc.Depth = 6;
            desc.Format = TextureFormat::Depth32;
            desc.Levels = 1;
            shadow.ShadowMap = mRHI->CreateTexture(desc);

            shadow.SRV = mRHI->CreateView(shadow.ShadowMap, ViewType::ShaderResource, ViewDimension::TextureCube, TextureFormat::R32Float);
            for (int i = 0; i < 6; i++) {
                shadow.DepthViews[i] = mRHI->CreateView(shadow.ShadowMap, ViewType::DepthTarget, ViewDimension::TextureCube, TextureFormat::Depth32, VIEW_ALL_MIPS, i);
            }
            
            scene.PointLights[i].ShadowCubemap = shadow.SRV->GetDescriptor().Index;
            mPointLightShadows.push_back(shadow);
        }
    }
    for (int i = 0; i < scene.SpotLights.size(); i++) {
        if (scene.SpotLights[i].CastShadows) {
            SpotLightShadow shadow;
            shadow.Parent = &scene.SpotLights[i];
        
            TextureDesc desc;
            desc.Name = "Spot Light Shadow Map";
            desc.Usage = TextureUsage::DepthTarget;
            desc.Width = SPOT_LIGHT_SHADOW_DIMENSION;
            desc.Height = SPOT_LIGHT_SHADOW_DIMENSION;
            desc.Depth = 1;
            desc.Format = TextureFormat::Depth32;
            desc.Levels = 1;
            shadow.ShadowMap = mRHI->CreateTexture(desc);

            shadow.SRV = mRHI->CreateView(shadow.ShadowMap, ViewType::ShaderResource, ViewDimension::Texture, TextureFormat::R32Float);
            shadow.DSV = mRHI->CreateView(shadow.ShadowMap, ViewType::DepthTarget, ViewDimension::Texture);
            
            scene.SpotLights[i].ShadowMap = shadow.SRV->GetDescriptor().Index;
            mSpotLightShadows.push_back(shadow);
        }
    }
}

void Shadows::Render(const Frame& frame, Scene& scene)
{
    frame.CommandBuffer->BeginMarker("Shadows");

    // CSM
    {
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
        frame.CommandBuffer->SetGraphicsPipeline(mCascadePipeline);
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

    // Point shadows
    {
        frame.CommandBuffer->BeginMarker("Point Shadows");
        frame.CommandBuffer->SetGraphicsPipeline(mPointPipeline);
        for (auto& light : mPointLightShadows) {
            float aspect = (float)POINT_LIGHT_SHADOW_DIMENSION / (float)POINT_LIGHT_SHADOW_DIMENSION;
            float nearPlane = 1.0f;
            float farPlane = 25.0f;
            glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), aspect, nearPlane, farPlane); 

            Vector<glm::mat4> shadowTransforms;
            shadowTransforms.push_back(glm::lookAt(light.Parent->Position, light.Parent->Position + glm::vec3( 1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
            shadowTransforms.push_back(glm::lookAt(light.Parent->Position, light.Parent->Position + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
            shadowTransforms.push_back(glm::lookAt(light.Parent->Position, light.Parent->Position + glm::vec3( 0.0, 1.0, 0.0), glm::vec3(0.0, 0.0,  1.0)));
            shadowTransforms.push_back(glm::lookAt(light.Parent->Position, light.Parent->Position + glm::vec3( 0.0,-1.0, 0.0), glm::vec3(0.0, 0.0, -1.0)));
            shadowTransforms.push_back(glm::lookAt(light.Parent->Position, light.Parent->Position + glm::vec3( 0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0)));
            shadowTransforms.push_back(glm::lookAt(light.Parent->Position, light.Parent->Position + glm::vec3( 0.0, 0.0,-1.0), glm::vec3(0.0, -1.0, 0.0)));

            for (int i = 0; i < 6; i++) {
                frame.CommandBuffer->Barrier(light.ShadowMap, ResourceLayout::DepthWrite);
                frame.CommandBuffer->SetRenderTargets({}, light.DepthViews[i]);
                frame.CommandBuffer->ClearDepth(light.DepthViews[i]);
                frame.CommandBuffer->SetViewport(0, 0, POINT_LIGHT_SHADOW_DIMENSION, POINT_LIGHT_SHADOW_DIMENSION);
                frame.CommandBuffer->SetTopology(Topology::TriangleList);
                std::function<void(Frame frame, GLTFNode*, GLTF* model, glm::mat4 transform)> drawNode = [&](Frame frame, GLTFNode* node, GLTF* model, glm::mat4 transform) {
                    if (!node) {
                        return;
                    }

                    glm::mat4 globalTransform = transform * node->Transform;
                    for (GLTFPrimitive primitive : node->Primitives) {
                        if (!Camera::IsBoxInFrustum(shadowProj * shadowTransforms[i], primitive.AABB, globalTransform))
                            continue;

                        struct PushConstants {
                            glm::mat4 transform;
                            glm::mat4 view;
                            glm::mat4 proj;
                            glm::vec4 lightPos;
                        } Constants = {
                            globalTransform,
                            shadowTransforms[i],
                            shadowProj,
                            glm::vec4(light.Parent->Position, 1.0)
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

                frame.CommandBuffer->Barrier(light.ShadowMap, ResourceLayout::Shader);
            }
        }
        frame.CommandBuffer->EndMarker();
    }

    // Spot shadows
    {
        frame.CommandBuffer->BeginMarker("Spot Shadows");
        frame.CommandBuffer->SetGraphicsPipeline(mSpotPipeline);
        for (auto& light : mSpotLightShadows) {
            float aspect = (float)SPOT_LIGHT_SHADOW_DIMENSION / (float)SPOT_LIGHT_SHADOW_DIMENSION;
            float nearPlane = 1.0f;
            float farPlane = 25.0f;

            glm::mat4 shadowProj = glm::perspective(light.Parent->OuterRadius * 2, aspect, nearPlane, farPlane); 
            glm::mat4 shadowView = glm::lookAt(light.Parent->Position, light.Parent->Position + light.Parent->Direction, glm::vec3(0.0f, 1.0f, 0.0f));
            light.Parent->LightView = shadowView;
            light.Parent->LightProj = shadowProj;

            frame.CommandBuffer->Barrier(light.ShadowMap, ResourceLayout::DepthWrite);
            frame.CommandBuffer->SetRenderTargets({}, light.DSV);
            frame.CommandBuffer->ClearDepth(light.DSV);
            frame.CommandBuffer->SetViewport(0, 0, SPOT_LIGHT_SHADOW_DIMENSION, SPOT_LIGHT_SHADOW_DIMENSION);
            frame.CommandBuffer->SetTopology(Topology::TriangleList);
            std::function<void(Frame frame, GLTFNode*, GLTF* model, glm::mat4 transform)> drawNode = [&](Frame frame, GLTFNode* node, GLTF* model, glm::mat4 transform) {
                if (!node) {
                    return;
                }

                glm::mat4 globalTransform = transform * node->Transform;
                for (GLTFPrimitive primitive : node->Primitives) {
                    if (!Camera::IsBoxInFrustum(shadowProj * shadowView, primitive.AABB, globalTransform))
                        continue;
                    struct PushConstants {
                        glm::mat4 transform;
                        glm::mat4 view;
                        glm::mat4 proj;
                    } Constants = {
                        globalTransform,
                        shadowView,
                        shadowProj
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
            frame.CommandBuffer->Barrier(light.ShadowMap, ResourceLayout::Shader);
        }
        frame.CommandBuffer->EndMarker();
    }

    frame.CommandBuffer->EndMarker();
}

void Shadows::UI(const Frame& frame)
{
    Vector<::Ref<RenderPassIO>> cascades = {
        PassManager::Get("ShadowCascade0"),
        PassManager::Get("ShadowCascade1"),
        PassManager::Get("ShadowCascade2"),
        PassManager::Get("ShadowCascade3"),
    };

    if (ImGui::TreeNodeEx("Shadows", ImGuiTreeNodeFlags_Framed)) {
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

void Shadows::UpdateCascades(const Scene& scene)
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
        glm::vec3 shadowCameraPos = center - scene.Sun.Direction;

        glm::mat4 lightView = glm::lookAt(shadowCameraPos, center, up);
        glm::mat4 lightProjection = glm::ortho(
            minBounds.x,
            maxBounds.x,
            minBounds.y,
            maxBounds.y,
            minBounds.z,
            maxBounds.z
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
