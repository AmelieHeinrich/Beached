//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-03 18:52:15
//

struct FragmentIn
{
    float4 Position : SV_Position;
    float2 UV : TEXCOORD;
};

float4 PSMain(FragmentIn Input) : SV_Target
{
    return float4(Input.UV, 0.0, 1.0);
}
