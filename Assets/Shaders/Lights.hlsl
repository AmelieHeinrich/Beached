//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-07 17:41:08
//

struct PointLight
{
    float3 Position;
    float Radius;
    float4 Color;
};

struct LightData
{
    PointLight Lights[1024];
    int LightCount;
};