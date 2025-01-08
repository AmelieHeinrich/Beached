//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2025-01-04 06:40:39
//

float ComputePCF(
    Texture2D<float> ShadowMap,
    SamplerComparisonState comparisonSampler,
    float4 WorldSpacePosition,
    float3 LightDirection,
    float4x4 LightView,
    float4x4 LightProj,
    float bias,
    int kernelSize)
{
    // Transform world-space position into light space
    float4 lightSpacePosition = mul(LightView, WorldSpacePosition);
    float4 ndcPosition = mul(LightProj, lightSpacePosition);
    ndcPosition.xyz /= ndcPosition.w;

    // Compute shadow map UV coordinates
    float2 shadowUV = ndcPosition.xy * 0.5 + 0.5;
    shadowUV.y = 1.0 - shadowUV.y;

    // Check if outside the light frustum
    if (ndcPosition.z > 1.0)
        return 1.0;

    // Compute texel size for PCF kernel sampling
    uint shadowWidth, shadowHeight;
    ShadowMap.GetDimensions(shadowWidth, shadowHeight);
    float2 texelSize = 1.0 / float2(shadowWidth, shadowHeight);

    // Perform PCF with the comparison sampler
    float shadow = 0.0;
    int sampleCount = 0;

    for (int x = -kernelSize; x <= kernelSize; x++) {
        for (int y = -kernelSize; y <= kernelSize; y++) {
            // Offset UV coordinates for sampling
            float2 offsetUV = shadowUV + float2(x, y) * texelSize;

            // Use the comparison sampler to perform the depth test
            shadow += ShadowMap.SampleCmpLevelZero(comparisonSampler, offsetUV, ndcPosition.z - bias);
            sampleCount++;
        }
    }
    shadow /= sampleCount;
    
    return shadow;
}

float ComputePCFPoint(
    TextureCube<float> ShadowMap,
    SamplerComparisonState comparisonSampler,
    float4 WorldSpacePosition,
    float3 LightPosition,
    float bias,
    int kernelSize)
{
    float3 fragToLight = WorldSpacePosition.xyz - LightPosition;
    float currentDepth = length(fragToLight);
 
    float closestDepth = ShadowMap.SampleCmpLevelZero(comparisonSampler, fragToLight, 1.0).r;
    closestDepth *= 25.0;

    float myBias = 0.05;
    float shadow = currentDepth - myBias > closestDepth ? 0.0 : 1.0;

    return shadow;
}
