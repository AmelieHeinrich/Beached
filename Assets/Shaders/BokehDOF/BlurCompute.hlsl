//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2025-01-18 14:16:20
//

struct Data
{
    int Output;
    int3 Pad;
};

ConstantBuffer<Data> Constants : register(b0);

[numthreads(8, 8, 1)]
void CSMain(uint3 TID : SV_DispatchThreadID)
{
    RWTexture2D<float> coc = ResourceDescriptorHeap[Constants.Output];

    uint width, height;
    coc.GetDimensions(width, height);
    if (TID.x > width || TID.y > height)
        return;

    // 5x5 box
    float nearSample = 0.0;
    int sampleCount = 0;
    for (int x = -1; x < 1; x++) {
        for (int y = -1; y < 1; y++) {
            nearSample += coc[TID.xy + uint2(x, y)];
            sampleCount++;
        }
    }
    nearSample /= sampleCount;
    coc[TID.xy] = nearSample;
}