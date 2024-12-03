//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-03 18:52:15
//

struct FragmentIn
{
    float4 Position : SV_Position;
    float3 Color : COLOR;
};

float4 PSMain(FragmentIn Input) : SV_Target
{
    return float4(Input.Color, 1.0);
}
