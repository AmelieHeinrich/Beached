//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-08 01:43:41
//

struct VertexIn
{
    float3 Position : POSITION;
    float2 UV : TEXCOORD;
    float3 Normal : NORMAL;
};

struct Settings
{
    column_major float4x4 Transform;
    column_major float4x4 LightView;
    column_major float4x4 LightProj;  
};

ConstantBuffer<Settings> PushConstants : register(b0);

float4 VSMain(VertexIn Input) : SV_Position
{
    // Add a homogeneous coordinate for transformation
    float4 worldPosition = mul(PushConstants.Transform, float4(Input.Position, 1.0)); // Model-space to World-space
    float4 lightViewPosition = mul(PushConstants.LightView, worldPosition);          // World-space to Light-view-space
    float4 clipSpacePosition = mul(PushConstants.LightProj, lightViewPosition);      // Light-view-space to Clip-space
    return clipSpacePosition;
}
