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

struct SpotLight
{
    glm::vec3 Position;
    float Radius;
    glm::vec3 Direction;
    int ShadowMap;
    glm::vec3 Color;
    bool CastShadows;
    float OuterRadius;
    glm::vec3 Pad;

    glm::mat4 LightView;
    glm::mat4 LightProj;
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
    int SpotLightSRV;
    int SpotLightCount;

    int UseSun;
    glm::ivec3 Pad;
};

class Scene
{
public:
    Camera Camera;
    Box SceneOBB;

    Vector<SpotLight> SpotLights;
    Vector<PointLight> PointLights;
    DirectionalLight Sun;

    Vector<RaytracingInstance> Instances;
    Buffer::Ref InstanceBuffer;
    TLAS::Ref TLAS;

    Vector<Asset::Handle> Models;
    Array<Buffer::Ref, FRAMES_IN_FLIGHT> LightBuffer;
    Array<Buffer::Ref, FRAMES_IN_FLIGHT> PointLightBuffer;
    Array<Buffer::Ref, FRAMES_IN_FLIGHT> SpotLightBuffer;

    void Init(RHI::Ref rhi);
    void BakeBLAS(RHI::Ref rhi);
    void BakeTLAS(RHI::Ref rhi);
    void Update(const Frame& frame, UInt32 frameIndex);
private:
    LightData mData;
};
