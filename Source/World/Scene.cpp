//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-07 17:09:24
//

#include <World/Scene.hpp>

#include <RHI/Uploader.hpp>

void Scene::Bake(RHI::Ref rhi)
{
    std::function<void(GLTFNode*)> updateNode = [&](GLTFNode* node) {
        if (!node) {
            return;
        }

        for (auto& primitive : node->Primitives) {
            Instances.push_back(primitive.Instance);
        }

        if (!node->Children.empty()) {
            for (GLTFNode* child : node->Children) {
                updateNode(child);
            }
        }
    };
    for (auto& model : Models) {
        updateNode(model->Model.Root);
    }

    InstanceBuffer = rhi->CreateBuffer(Instances.size() * sizeof(RaytracingInstance), sizeof(RaytracingInstance), BufferType::Constant, "Scene Instance Buffers");
    InstanceBuffer->CopyMapped(Instances.data(), Instances.size() * sizeof(RaytracingInstance));

    TLAS = rhi->CreateTLAS(InstanceBuffer, Instances.size(), "Scene TLAS");
    Uploader::EnqueueAccelerationStructureBuild(TLAS);

    for (int i = 0; i < FRAMES_IN_FLIGHT; i++) {
        LightBuffer[i] = rhi->CreateBuffer(65536, 0, BufferType::Constant, "Light CBV");
        LightBuffer[i]->BuildCBV();
    }
}

void Scene::Update(const Frame& frame, UInt32 frameIndex)
{
    // Update light buffer
    mData.LightCount = PointLights.size();
    mData.Sun = Sun;
    if (PointLights.size() > 0) {
        memcpy(mData.Lights.data(), PointLights.data(), PointLights.size() * sizeof(PointLight));
    }
    LightBuffer[frameIndex]->CopyMapped(&mData, sizeof(mData));

    // Update instances
    Instances.clear();
    std::function<void(GLTFNode*)> updateNode = [&](GLTFNode* node) {
        if (!node) {
            return;
        }

        for (auto& primitive : node->Primitives) {
            Instances.push_back(primitive.Instance);
        }

        if (!node->Children.empty()) {
            for (GLTFNode* child : node->Children) {
                updateNode(child);
            }
        }
    };
    for (auto& model : Models) {
        updateNode(model->Model.Root);
    }
    InstanceBuffer->CopyMapped(Instances.data(), Instances.size() * sizeof(RaytracingInstance));

    // Update TLAS
    // frame.CommandBuffer->UpdateTLAS(TLAS, InstanceBuffer, Instances.size());
    // frame.CommandBuffer->UAVBarrier(TLAS);
}
