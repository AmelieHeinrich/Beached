//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2025-01-19 07:16:59
//

#include "Assets/Shaders/Color.hlsl"

#define NUM_HISTOGRAM_BINS 256

struct Data
{
    int Input;
    int Output;
    float MinLogLuminance;
    float MinLogLuminanceInv;
};

ConstantBuffer<Data> Constants : register(b0);
groupshared uint HistogramShared[NUM_HISTOGRAM_BINS];

uint HDRToHistogramBin(float3 hdrColor)
{
    float luminance = GetLuminance(hdrColor);
    if (luminance < 0.001) {
        return 0;
    }
    
    float logLuminance = saturate((log2(luminance) - Constants.MinLogLuminance) * Constants.MinLogLuminanceInv);
    return (uint)(logLuminance * 254.0 + 1.0);
}

[numthreads(16, 16, 1)]
void CSMain(uint GID : SV_GroupID, uint3 TID : SV_DispatchThreadID)
{
    //
    Texture2D<float4> inputBuffer = ResourceDescriptorHeap[Constants.Input];
    RWByteAddressBuffer histogram = ResourceDescriptorHeap[Constants.Output];
    
    uint width, height;
    inputBuffer.GetDimensions(width, height);

    //
    HistogramShared[GID] = 0;

    GroupMemoryBarrierWithGroupSync();
    if (TID.x < width && TID.y < height) {
        float3 color = inputBuffer.Load(int3(TID.xy, 0)).rgb;
        uint binIndex = HDRToHistogramBin(color);
        InterlockedAdd(HistogramShared[binIndex], 1);
    }

    GroupMemoryBarrierWithGroupSync();

    histogram.InterlockedAdd(GID * 4, HistogramShared[GID]);
}
