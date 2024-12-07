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

struct LightData
{
    Array<PointLight, 1024> Lights;
    int LightCount;
};

class Scene
{
public:
    Camera Camera;
    Vector<PointLight> PointLights;

    Vector<Asset::Handle> Models;
    Array<Buffer::Ref, FRAMES_IN_FLIGHT> LightBuffer;

    void Init(RHI::Ref rhi);
    void Update(UInt32 frameIndex);
private:
    LightData mData;
};
