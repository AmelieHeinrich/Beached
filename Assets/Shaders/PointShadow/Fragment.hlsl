//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-08 01:45:44
//

struct Settings
{
    column_major float4x4 Transform;
    column_major float4x4 LightView;
    column_major float4x4 LightProj;
    float4 LightPos;
};

struct FragmentIn
{
    float4 ClipPosition : SV_Position;
    float4 WorldPosition : POSITION;
};

ConstantBuffer<Settings> PushConstants : register(b0);

float PSMain(FragmentIn input) : SV_Depth
{
    float lightDistance = length(input.WorldPosition.xyz - PushConstants.LightPos.xyz);
    lightDistance = lightDistance / 25.0;
    return lightDistance;
}
