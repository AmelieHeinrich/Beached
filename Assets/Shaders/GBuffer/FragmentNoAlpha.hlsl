//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-21 04:36:05
//

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

ConstantBuffer<Settings> PushConstants : register(b0);

void PSMain(FragmentIn Input)
{
}
