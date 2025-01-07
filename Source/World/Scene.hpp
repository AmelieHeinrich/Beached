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
    glm::vec3 Color;
    int ShadowCubemap;

    bool CastShadows;
    glm::ivec3 Pad;
};

struct DirectionalLight
{
    glm::vec3 Direction;
    float Strength;
    glm::vec4 Color;
};

struct LightData
{
    DirectionalLight Sun;

    int PointLightSRV;
    int PointLightCount;
    int UseSun;
    int Pad;
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
    Array<Buffer::Ref, FRAMES_IN_FLIGHT> PointLightBuffer;

    void Init(RHI::Ref rhi);
    void BakeBLAS(RHI::Ref rhi);
    void BakeTLAS(RHI::Ref rhi);
    void Update(const Frame& frame, UInt32 frameIndex);
private:
    LightData mData;
};
