//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-07 17:09:24
//

#include <World/Scene.hpp>

void Scene::Init(RHI::Ref rhi)
{
    for (int i = 0; i < FRAMES_IN_FLIGHT; i++) {
        LightBuffer[i] = rhi->CreateBuffer(65536, 0, BufferType::Constant, "Light CBV");
        LightBuffer[i]->BuildCBV();
    }
}

void Scene::Update(UInt32 frameIndex)
{
    mData.LightCount = PointLights.size();
    mData.Sun = Sun;
    if (PointLights.size() > 0) {
        memcpy(mData.Lights.data(), PointLights.data(), PointLights.size() * sizeof(PointLight));
    }

    LightBuffer[frameIndex]->CopyMapped(&mData, sizeof(mData));
}
