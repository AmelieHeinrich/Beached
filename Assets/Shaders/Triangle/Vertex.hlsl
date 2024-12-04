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

struct Camera
{
    column_major float4x4 View;
    column_major float4x4 Projection;
};

struct Settings
{
    int CameraIndex;
};

ConstantBuffer<Settings> PushConstants : register(b0);

VertexOut VSMain(VertexIn Input)
{
    ConstantBuffer<Camera> Cam = ResourceDescriptorHeap[PushConstants.CameraIndex];

    VertexOut Output = (VertexOut)0;
    Output.Position = float4(Input.Position, 1.0);
    Output.Position = mul(Cam.View, Output.Position);
    Output.Position = mul(Cam.Projection, Output.Position);
    Output.Color = Input.Color;
    return Output;
}
