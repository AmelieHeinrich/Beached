//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2025-01-04 06:40:39
//

float ComputePCF(
    Texture2D<float> ShadowMap,
    SamplerState sampler,
    float4 WorldSpacePosition,
    float3 Normal,
    float3 LightDirection,
    float4x4 LightView,
    float4x4 LightProj,
    float bias,
    int kernelSize)
{
    float4 lightSpacePosition = mul(LightView, WorldSpacePosition);
    float4 ndcPosition = mul(LightProj, lightSpacePosition);
    ndcPosition.xyz /= ndcPosition.w;

    float2 shadowUV = ndcPosition.xy * 0.5 + 0.5;
    shadowUV.y = 1.0 - shadowUV.y;

    float shadowMapDepth = ShadowMap.Sample(sampler, shadowUV);
    float currentDepth = ndcPosition.z;
    if (currentDepth > 1.0)
        return 1.0;

    uint shadowWidth, shadowHeight;
    ShadowMap.GetDimensions(shadowWidth, shadowHeight);
    float2 texelSize = 1.0 / float2(shadowWidth, shadowHeight);
    
    //float shadow = 0.0;
    //for (int x = -kernelSize; x <= kernelSize; x++) {
    //    for (int y = -kernelSize; y <= kernelSize; y++) {
    //        float pcfDepth = ShadowMap.Sample(sampler, float2(shadowUV + float2(x, y) * texelSize)).r;
    //        shadow += currentDepth > pcfDepth ? 0.0 : 1.0;
    //    }
    //}
    //shadow /= (kernelSize + 2) * (kernelSize + 2);

    float shadowDepth = ShadowMap.Sample(sampler, shadowUV).r;
    float shadow = currentDepth - 0.001 > shadowDepth ? 0.0 : 1.0;
    return shadow;
}
