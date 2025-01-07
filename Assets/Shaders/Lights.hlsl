//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-07 17:41:08
//

struct PointLight
{
    float3 Position;
    float Radius;
    float3 Color;
    int ShadowCubemap;

    bool CastShadows;
    int3 Pad;
};

struct DirectionalLight
{
    float3 Direction;
    float Strength;
    float4 Color;
};

struct LightData
{
    // Sun
    DirectionalLight Sun;

    // Point lights
    int PointLightBuffer;
    int PointLightCount;
    int UseSun;
    int Pad;

    // TODO: Spot lights
};