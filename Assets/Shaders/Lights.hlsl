//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-07 17:41:08
//

struct SpotLight
{
    float3 Position;
    float Radius;
    float3 Direction;
    int ShadowMap;
    float3 Color;
    bool CastShadows;
    float OuterRadius;
    float3 Pad;

    column_major float4x4 LightView;
    column_major float4x4 LightProj;
};

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
    DirectionalLight Sun;

    int PointLightSRV;
    int PointLightCount;
    int SpotLightSRV;
    int SpotLightCount;

    int UseSun;
    int3 Pad;
};