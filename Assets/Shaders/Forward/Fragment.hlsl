//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-03 18:52:15
//

#include "Assets/Shaders/Lights.hlsl"
#include "Assets/Shaders/Camera.hlsl"

struct FragmentIn
{
    float4 Position : SV_Position;
    float2 UV : TEXCOORD;
    float3 Normal : NORMAL;
    float3 FragPosWorld : POSITION;
    float3 FragPosView : POSITION1;
};

struct Settings
{
    // CBVs
    int CameraIndex;
    int ModelIndex;
    int LightIndex;
    int TextureIndex;
    int SamplerIndex;
    int3 Pad;
};

ConstantBuffer<Settings> PushConstants : register(b0);

float3 CalculatePoint(PointLight Light, FragmentIn Input, float3 Albedo)
{
    ConstantBuffer<Camera> Cam = ResourceDescriptorHeap[PushConstants.CameraIndex];

    // TODO: Normal maps.
    float3 LightDirection = normalize(Light.Position - Input.FragPosWorld);
    float Distance = length(Light.Position - Input.FragPosWorld);
    float Attenuation = 1.0 / (Distance * Distance);

    float NdotL = max(dot(Input.Normal, LightDirection) * Light.Radius, 0.04);
    return (NdotL * Albedo * Attenuation * Light.Color.xyz);
}

float3 CalculateSun(DirectionalLight Light, FragmentIn Input, float3 Albedo)
{
    ConstantBuffer<Camera> Cam = ResourceDescriptorHeap[PushConstants.CameraIndex];

    // TODO: Normal maps.
    float NdotL = max(dot(Input.Normal, -Light.Direction), 0.04);
    return (NdotL * Albedo * Light.Strength * Light.Color.xyz);
}

float4 GetCascadeColor(int layer)
{
    switch (layer) {
        case 0:
            return float4(1.0f, 0.0f, 0.0f, 1.0f);
        case 1:
            return float4(0.0f, 1.0f, 0.0f, 1.0f);
        case 2:
            return float4(0.0f, 0.0f, 1.0f, 1.0f);
        case 3:
            return float4(1.0f, 0.0f, 1.0f, 1.0f);
    }
    return 0.0f;
}

float4 PSMain(FragmentIn Input) : SV_Target
{
    ConstantBuffer<LightData> Lights = ResourceDescriptorHeap[PushConstants.LightIndex];
    Texture2D Albedo = ResourceDescriptorHeap[PushConstants.TextureIndex];
    SamplerState Sampler = SamplerDescriptorHeap[PushConstants.SamplerIndex];

    int layer = -1;
    for (int i = 0; i < 4; i++) {
        if (abs(Input.FragPosView.z) < SHADOW_CASCADE_LEVELS[i]) {
            layer = i;
            break;
        }
    }
    if (layer == -1) {
        layer = 3;
    }

    float4 Color = Albedo.Sample(Sampler, Input.UV);
    if (Color.a < 0.1)
        discard;
    
    float3 Lo = CalculateSun(Lights.Sun, Input, Color.xyz);
    for (int i = 0; i < Lights.LightCount; i++) {
        Lo += CalculatePoint(Lights.Lights[i], Input, Color.xyz);
    }
    return float4(Lo, 1.0);
}
