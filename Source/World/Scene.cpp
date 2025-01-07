//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-07 17:09:24
//

#include <World/Scene.hpp>
#include <Settings.hpp>

#include <RHI/Uploader.hpp>

void Scene::BakeBLAS(RHI::Ref rhi)
{
    for (auto& model : Models) {
        model->Model.TraverseNode(model->Model.Root, [&](GLTFNode* node){
            for (auto& primitive : node->Primitives) {
                Uploader::EnqueueAccelerationStructureBuild(primitive.GeometryStructure);
            }
        });
    }
}

void Scene::BakeTLAS(RHI::Ref rhi)
{
    for (auto& model : Models) {
        model->Model.TraverseNode(model->Model.Root, [&](GLTFNode* node){
            for (auto& primitive : node->Primitives) {
                Instances.push_back(primitive.Instance);
            }
        });
    }

    InstanceBuffer = rhi->CreateBuffer(Instances.size() * sizeof(RaytracingInstance), sizeof(RaytracingInstance), BufferType::Constant, "Scene Instance Buffers");
    InstanceBuffer->CopyMapped(Instances.data(), Instances.size() * sizeof(RaytracingInstance));

    TLAS = rhi->CreateTLAS(InstanceBuffer, Instances.size(), "Scene TLAS");
    Uploader::EnqueueAccelerationStructureBuild(TLAS);
}

void Scene::Init(RHI::Ref rhi)
{
    for (int i = 0; i < FRAMES_IN_FLIGHT; i++) {
        LightBuffer[i] = rhi->CreateBuffer(256, 0, BufferType::Constant, "Light CBV");
        LightBuffer[i]->BuildCBV();

        PointLightBuffer[i] = rhi->CreateBuffer(16384, sizeof(PointLight), BufferType::Constant, "Point Light UAV");
        PointLightBuffer[i]->BuildSRV();
    }

    std::function<void(GLTFNode*, glm::mat4 transform)> traverseScene = [&](GLTFNode* node, glm::mat4 transform) {
        if (!node) {
            return;
        }

        glm::mat4 globalTransform = transform * node->Transform;
        for (GLTFPrimitive primitive : node->Primitives) {
            Box& box = primitive.AABB;
            glm::vec3 corners[8] = {
                glm::vec3(box.Min.x, box.Min.y, box.Min.z),
                glm::vec3(box.Min.x, box.Min.y, box.Max.z),
                glm::vec3(box.Min.x, box.Max.y, box.Min.z),
                glm::vec3(box.Min.x, box.Max.y, box.Max.z),
                glm::vec3(box.Max.x, box.Min.y, box.Min.z),
                glm::vec3(box.Max.x, box.Min.y, box.Max.z),
                glm::vec3(box.Max.x, box.Max.y, box.Min.z),
                glm::vec3(box.Max.x, box.Max.y, box.Max.z),
            };

            // Transform each corner by the matrix
            for (auto& corner : corners) {
                corner = glm::vec3(node->Transform * glm::vec4(corner, 1.0f));
                this->SceneOBB.Min = glm::min(corner, this->SceneOBB.Min);
                this->SceneOBB.Max = glm::max(corner, this->SceneOBB.Max);
            }
        }
        if (!node->Children.empty()) {
            for (GLTFNode* child : node->Children) {
                traverseScene(child, globalTransform);
            }
        }
    };

    // Compute scene OBB
    for (auto& model : Models) {
        traverseScene(model->Model.Root, glm::mat4(1.0f));
    }
}

void Scene::Update(const Frame& frame, UInt32 frameIndex)
{
    // Update light buffer
    mData.Sun = Sun;
    mData.PointLightSRV = PointLightBuffer[frameIndex]->SRV();
    mData.PointLightCount = PointLights.size();
    mData.UseSun = Settings::Get().SceneUseSun;

    PointLightBuffer[frameIndex]->CopyMapped(PointLights.data(), PointLights.size() * sizeof(PointLight));
    LightBuffer[frameIndex]->CopyMapped(&mData, sizeof(LightData));
}
