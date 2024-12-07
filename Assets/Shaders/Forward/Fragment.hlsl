//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-03 18:52:15
//

struct FragmentIn
{
    float4 Position : SV_Position;
    float2 UV : TEXCOORD;
    float3 Normal : NORMAL;
};

struct Settings
{
    int CameraIndex;
    int ModelIndex;
    int TextureIndex;
    int SamplerIndex;
};

ConstantBuffer<Settings> PushConstants : register(b0);

float4 PSMain(FragmentIn Input) : SV_Target
{
    Texture2D Albedo = ResourceDescriptorHeap[PushConstants.TextureIndex];
    SamplerState Sampler = SamplerDescriptorHeap[PushConstants.SamplerIndex];

    float4 Color = Albedo.Sample(Sampler, Input.UV);
    if (Color.a < 0.1)
        discard;
    return Color;
}
