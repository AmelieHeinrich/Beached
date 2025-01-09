//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2025-01-04 06:40:39
//

static const float3 PCF_POISSON_DISK[20] = {
   float3( 1,  1,  1), float3( 1, -1,  1), float3(-1, -1,  1), float3(-1,  1,  1), 
   float3( 1,  1, -1), float3( 1, -1, -1), float3(-1, -1, -1), float3(-1,  1, -1),
   float3( 1,  1,  0), float3( 1, -1,  0), float3(-1, -1,  0), float3(-1,  1,  0),
   float3( 1,  0,  1), float3(-1,  0,  1), float3( 1,  0, -1), float3(-1,  0, -1),
   float3( 0,  1,  1), float3( 0, -1,  1), float3( 0, -1, -1), float3( 0,  1, -1)
};

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
    SamplerState comparisonSampler,
    float4 WorldSpacePosition,
    float3 CameraPosition,
    float3 LightPosition,
    int kernelSize)
{
    float3 fragToLight = WorldSpacePosition.xyz - LightPosition;
    fragToLight.y = -fragToLight.y;

    float currentDepth = length(fragToLight);

    float shadow = 0.0;
    float bias = 0.05;

    // Define an offset array for 2x2 sampling (cheap PCF)
    float3 offset[4] = {
        float3( 0.5,  0.5, 0.0),
        float3(-0.5,  0.5, 0.0),
        float3( 0.5, -0.5, 0.0),
        float3(-0.5, -0.5, 0.0)
    };

    // Sample multiple points within the kernel
    for (int i = 0; i < 4; ++i)
    {
        float3 sampleDirection = fragToLight + offset[i] * 0.01; // Small jitter for sampling
        float closestDepth = ShadowMap.Sample(comparisonSampler, sampleDirection).r;
        closestDepth *= 25;

        // Accumulate shadow contribution
        if (currentDepth - bias <= closestDepth)
        {
            shadow += 0.25; // Weight of each sample (1/4 for 2x2 PCF)
        }
    }

    return shadow;
}
