//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-03 18:49:03
//

struct VertexIn
{
    float3 Position : POSITION;
    float3 Color : COLOR;
};

struct VertexOut
{
    float4 Position : SV_Position;
    float3 Color : COLOR;
};

VertexOut VSMain(VertexIn Input)
{
    VertexOut Output = (VertexOut)0;
    Output.Position = float4(Input.Position, 1.0);
    Output.Color = Input.Color;
    return Output;
}
