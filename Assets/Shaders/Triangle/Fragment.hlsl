//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-03 18:52:15
//

struct FragmentIn
{
    float4 Position : SV_Position;
    float2 UV : TEXCOORD;
};

struct Settings
{
    int CameraIndex;
    int TextureIndex;
    int SamplerIndex;
};

ConstantBuffer<Settings> PushConstants : register(b0);

float4 PSMain(FragmentIn Input) : SV_Target
{
    Texture2D Albedo = ResourceDescriptorHeap[PushConstants.TextureIndex];
    SamplerState Sampler = SamplerDescriptorHeap[PushConstants.SamplerIndex];

    return Albedo.Sample(Sampler, Input.UV);
}
