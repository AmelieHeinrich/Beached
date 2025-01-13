//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2025-01-11 06:51:13
//

// Take depth texture input
// Take near and far texture as output
// If current depth <= focalpoint, put in near, otherwise put in far

#include "Assets/Shaders/Camera.hlsl"

struct Data
{
    int DepthInput;
    int ColorInput;
    int Sampler;
    int NearOutput;

    int FarOutput;
    float FocalPoint;
    float2 Pad;

    column_major float4x4 ProjInv;
};

ConstantBuffer<Data> Constants : register(b0);

[numthreads(8, 8, 1)]
void CSMain(uint3 TID : SV_DispatchThreadID)
{
    
}
