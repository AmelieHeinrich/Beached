//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-03-14 02:08:14
//

struct Constants
{
    int Temp;
};

ConstantBuffer<Constants> Settings : register(b0);

[numthreads(8, 8, 1)]
void CSMain(uint3 ThreadID : SV_DispatchThreadID)
{

}
