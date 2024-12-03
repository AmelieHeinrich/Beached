//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-03 18:49:03
//

static const float3 VERTICES[] = {
    float3(-0.5, -0.5, 0.0),
    float3( 0.5, -0.5, 0.0),
    float3( 0.0,  0.5, 0.0)
};

static const float3 COLORS[] = {
    float3( 1.0,  0.0, 0.0),
    float3( 0.0,  1.0, 0.0),
    float3( 0.0,  0.0, 1.0)
};

struct VertexOut
{
    float4 Position : SV_Position;
    float3 Color : COLOR;
};

VertexOut VSMain(uint VID : SV_VertexID)
{
    VertexOut Output = (VertexOut)0;
    Output.Position = float4(VERTICES[VID], 1.0);
    Output.Color = COLORS[VID];
    return Output;
}
