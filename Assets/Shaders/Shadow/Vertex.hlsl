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
    float4 worldPosition = mul(PushConstants.Transform, float4(Input.Position, 1.0f));
    float4 lightViewPosition = mul(PushConstants.LightView, worldPosition);
    return mul(PushConstants.LightProj, lightViewPosition);
}
