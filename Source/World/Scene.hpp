//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-06 00:09:15
//

#pragma once

#include <Asset/AssetManager.hpp>
#include <World/Camera.hpp>

struct PointLight
{
    glm::vec3 Position;
    float Radius;
    glm::vec4 Color; // W channel is for pad
};

struct DirectionalLight
{
    glm::vec3 Direction;
    float Strenght;
    glm::vec4 Color;
};

struct LightData
{
    Array<PointLight, 1024> Lights;
    int LightCount;
    glm::vec3 Pad;

    DirectionalLight Sun;
};

class Scene
{
public:
    Camera Camera;
    Box SceneOBB;

    Vector<PointLight> PointLights;
    DirectionalLight Sun;

    Vector<RaytracingInstance> Instances;
    Buffer::Ref InstanceBuffer;
    TLAS::Ref TLAS;

    Vector<Asset::Handle> Models;
    Array<Buffer::Ref, FRAMES_IN_FLIGHT> LightBuffer;

    void Init(RHI::Ref rhi);
    void BakeBLAS(RHI::Ref rhi);
    void BakeTLAS(RHI::Ref rhi);
    void Update(const Frame& frame, UInt32 frameIndex);
private:
    LightData mData;
};
