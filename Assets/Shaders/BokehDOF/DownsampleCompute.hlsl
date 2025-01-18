//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2025-01-18 13:00:09
//

struct Data
{
    int PointClamp;
    int LinearClamp;
    int ColorTexture;
    int COCTexture;

    int ColorX4;
    int COCMulFarX4;
    int COCX4;
    int Pad;
};

ConstantBuffer<Data> Constants : register(b0);

[numthreads(8, 8, 1)]
void CSMain(uint3 TID : SV_DispatchThreadID)
{
    
}
