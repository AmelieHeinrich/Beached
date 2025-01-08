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

struct VertexOut
{
    float4 ClipPosition : SV_Position;
    float4 WorldPosition : POSITION;
};

struct Settings
{
    column_major float4x4 Transform;
    column_major float4x4 LightView;
    column_major float4x4 LightProj;
    float4 LightPos;
};

ConstantBuffer<Settings> PushConstants : register(b0);

VertexOut VSMain(VertexIn Input)
{
    VertexOut output = (VertexOut)0;

    output.WorldPosition = mul(PushConstants.Transform, float4(Input.Position, 1.0f));
    
    float4 lightViewPosition = mul(PushConstants.LightView, output.WorldPosition);
    output.ClipPosition = mul(PushConstants.LightProj, lightViewPosition);

    return output;
}
