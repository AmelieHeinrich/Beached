//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-03 18:49:03
//

#include "Assets/Shaders/Camera.hlsl"

struct VertexIn
{
    float3 Position : POSITION;
    float2 UV : TEXCOORD;
    float3 Normal : NORMAL;
};

struct VertexOut
{
    float4 Position : SV_Position;
    float2 UV : TEXCOORD;
    float3 Normal : NORMAL;
    float3 FragPosWorld : POSITION;
    float3 FragPosView : POSITION1;
};

struct Model
{
    column_major float4x4 Transform;
};

struct Settings
{
    // CBVs
    int CameraIndex;
    int ModelIndex;
    int LightIndex;
    int TextureIndex;
    int SamplerIndex;
    int3 Pad;
};

ConstantBuffer<Settings> PushConstants : register(b0);

VertexOut VSMain(VertexIn Input)
{
    ConstantBuffer<Camera> Cam = ResourceDescriptorHeap[PushConstants.CameraIndex];
    ConstantBuffer<Model> Instance = ResourceDescriptorHeap[PushConstants.ModelIndex];

    float4 NDCPosition = float4(Input.Position, 1.0);
    NDCPosition = mul(Instance.Transform, NDCPosition);
    NDCPosition = mul(Cam.View, NDCPosition);
    NDCPosition = mul(Cam.Projection, NDCPosition);

    float4 WorldPosition = float4(Input.Position, 1.0);
    WorldPosition = mul(Instance.Transform, WorldPosition);

    float4 ViewPosition = float4(Input.Position, 1.0);
    ViewPosition = mul(Instance.Transform, ViewPosition);
    ViewPosition = mul(Cam.View, ViewPosition); 

    VertexOut Output = (VertexOut)0;
    Output.Position = NDCPosition;
    Output.UV = Input.UV;
    Output.Normal = Input.Normal;
    Output.FragPosWorld = WorldPosition.xyz;
    Output.FragPosView = ViewPosition.xyz;
    return Output;
}