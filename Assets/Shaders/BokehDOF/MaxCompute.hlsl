//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2025-01-18 14:16:20
//

struct Data
{
    int Input;
    int Output;
    int2 Pad;
};

ConstantBuffer<Data> Constants : register(b0);

[numthreads(8, 8, 1)]
void CSMain(uint3 TID : SV_DispatchThreadID)
{
    RWTexture2D<float2> coc = ResourceDescriptorHeap[Constants.Input];
    RWTexture2D<float> near = ResourceDescriptorHeap[Constants.Output];

    uint width, height;
    coc.GetDimensions(width, height);
    if (TID.x > width || TID.y > height)
        return;

    // 5x5 box
    float nearMax = 0.0;
    for (int x = -2; x < 2; x++) {
        for (int y = -2; y < 2; y++) {
            nearMax = max(nearMax, coc[TID.xy + uint2(x, y)].r);
        }
    }
    near[TID.xy] = nearMax;
}
