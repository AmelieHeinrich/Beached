//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-21 04:36:05
//

#include "Assets/Shaders/Lights.hlsl"
#include "Assets/Shaders/Camera.hlsl"
#include "Assets/Shaders/Cascade.hlsl"

#define DEFERRED 0

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
    
    // Model textures
    int TextureIndex;

    // Samplers
    int SamplerIndex;
};

struct FragmentOut
{
    float3 Normal : SV_Target0;
    float4 Albedo : SV_Target1;
};

ConstantBuffer<Settings> PushConstants : register(b0);

#if DEFERRED
FragmentOut PSMain(FragmentIn Input)
{
    Texture2D Albedo = ResourceDescriptorHeap[PushConstants.TextureIndex];
    SamplerState Sampler = SamplerDescriptorHeap[PushConstants.SamplerIndex];

    float4 Color = Albedo.Sample(Sampler, Input.UV);
    if (Color.a < 0.1)
        discard;
    
    FragmentOut output;
    output.Normal = Input.Normal;
    output.Albedo = Color;
    return output;
}
#else
void PSMain(FragmentIn Input)
{
    Texture2D Albedo = ResourceDescriptorHeap[PushConstants.TextureIndex];
    SamplerState Sampler = SamplerDescriptorHeap[PushConstants.SamplerIndex];

    float4 Color = Albedo.Sample(Sampler, Input.UV);
    if (Color.a < 0.5)
        discard;
}
#endif
