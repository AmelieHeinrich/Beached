//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2025-01-11 06:51:13
//

struct Data
{
    int PointClamp;
    int DepthBuffer;
    int Output;
    int Pad;

    float NearBegin;
    float NearEnd;
    float FarBegin;
    float FarEnd;

    float2 ProjParams;
    float2 Padding; 
};

ConstantBuffer<Data> Constants : register(b0);

float NDCToView(float depth)
{
    return -Constants.ProjParams.y / (depth + Constants.ProjParams.x);
}

[numthreads(8, 8, 1)]
void CSMain(uint3 TID : SV_DispatchThreadID)
{
    RWTexture2D<float2> output = ResourceDescriptorHeap[Constants.Output];
    Texture2D<float> depthBuffer = ResourceDescriptorHeap[Constants.DepthBuffer];
    SamplerState pointClampSampler = SamplerDescriptorHeap[Constants.PointClamp];

    uint depthWidth, depthHeight;
    depthBuffer.GetDimensions(depthWidth, depthHeight);

    if (TID.x >= depthWidth || TID.y >= depthHeight)
        return;

    float2 uv = TID.xy / float2(depthWidth, depthHeight);
    float depthNDC = depthBuffer.Sample(pointClampSampler, uv).r;
    float depth = -NDCToView(depthNDC);

    float nearCOC = 0.0f;
    if (depth < Constants.NearEnd)
        nearCOC = 1.0f / (Constants.NearBegin - Constants.NearEnd) * depth + -Constants.NearEnd / (Constants.NearBegin - Constants.NearEnd);
    else if (depth < Constants.NearBegin)
        nearCOC = 1.0f;
    nearCOC = saturate(nearCOC);

    float farCOC = 1.0f;
    if (depth < Constants.FarBegin)
        farCOC = 0.0f;
    else if (depth < Constants.FarEnd)
        farCOC = 1.0f / (Constants.FarEnd - Constants.FarBegin) * depth + -Constants.FarBegin / (Constants.FarEnd - Constants.FarBegin);
    farCOC = saturate(farCOC);

    output[TID.xy] = float2(nearCOC, farCOC);
}
