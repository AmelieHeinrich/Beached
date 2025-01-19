//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2025-01-19 07:21:54
//

float GetLuminance(float3 color)
{
    return dot(color, float3(0.2127f, 0.7152f, 0.0722f));
}
