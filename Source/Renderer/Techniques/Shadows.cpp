//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2025-01-07 12:43:59
//

#include <Renderer/Techniques/Shadows.hpp>
#include <Core/Logger.hpp>

Shadows::Shadows(RHI::Ref rhi)
    : RenderPass(rhi)
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
}

void Shadows::Render(const Frame& frame, Scene& scene)
{
    frame.CommandBuffer->BeginMarker("Shadows");

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
            shadowTransforms.push_back(glm::lookAt(light.Parent->Position, light.Parent->Position + glm::vec3( 0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)));
            shadowTransforms.push_back(glm::lookAt(light.Parent->Position, light.Parent->Position + glm::vec3( 0.0,-1.0, 0.0), glm::vec3(0.0, 0.0,-1.0)));
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

    // TODO: Spot shadows

    frame.CommandBuffer->EndMarker();
}

void Shadows::UI(const Frame& frame)
{

}
