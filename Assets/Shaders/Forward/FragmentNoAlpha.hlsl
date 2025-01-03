//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-03 18:52:15
//

#include "Assets/Shaders/Lights.hlsl"
#include "Assets/Shaders/Camera.hlsl"
#include "Assets/Shaders/Cascade.hlsl"

static const float AMBIENT = 0.01;

static const float4x4 BIAS_MATRIX = float4x4(
	0.5, 0.0, 0.0, 0.5,
	0.0, 0.5, 0.0, 0.5,
	0.0, 0.0, 1.0, 0.0,
	0.0, 0.0, 0.0, 1.0
);

struct FragmentIn
{
    float4 Position : SV_Position;
    float2 UV : TEXCOORD;
    float3 Normal : NORMAL;
    float3 FragPosWorld : POSITION;
    float3 FragPosView : POSITION1;
};

struct Settings
{
    // CBVs
    int CameraIndex;
    int ModelIndex;
    int LightIndex;
    int CascadeBufferIndex;
    
    // Model textures
    int TextureIndex;
    int NormalIndex;

    // Samplers
    int SamplerIndex;
    int ShadowSamplerIndex;

    // Acceleration structures
    int AccelStructure;
};

ConstantBuffer<Settings> PushConstants : register(b0);

float3 GetNormal(FragmentIn Input)
{
    if (PushConstants.NormalIndex == -1)
        return normalize(Input.Normal);

    Texture2D NormalTexture = ResourceDescriptorHeap[PushConstants.NormalIndex];
    SamplerState Sampler = SamplerDescriptorHeap[PushConstants.SamplerIndex];

    float3 tangentNormal = NormalTexture.Sample(Sampler, Input.UV.xy).rgb * 2.0 - 1.0;
    float3 normal = normalize(Input.Normal);

    float3 Q1 = ddx(Input.Position.xyz);
    float3 Q2 = ddy(Input.Position.xyz);
    float2 ST1 = ddx(Input.UV.xy);
    float2 ST2 = ddy(Input.UV.xy);

    // Add a small epsilon to avoid division by zero or extremely small values
    const float epsilon = 1e-6;

    // Compute T and handle potential degenerate cases
    float3 T = Q1 * ST2.y - Q2 * ST1.y;
    if (length(T) < epsilon) {
        T = float3(1.0, 0.0, 0.0); // Fallback to a default tangent
    }
    T = normalize(T);

    // Compute B and handle potential degenerate cases
    float3 B = cross(normal, T);
    if (length(B) < epsilon) {
        B = cross(normal, float3(0.0, 1.0, 0.0)); // Try using a different orthogonal vector
    }
    B = normalize(B);

    // Ensure N is normalized and handle small values
    float3 N = normal;
    if (length(N) < epsilon) {
        N = float3(0.0, 0.0, 1.0); // Fallback to a default normal
    }

    float3x3 TBN = float3x3(T, B, N);
    // Handle NaNs or invalid results from matrix multiplication
    float3 result = mul(tangentNormal, TBN);
    if (any(isnan(result)) || length(result) < epsilon) {
        result = float3(0.0, 0.0, 1.0); // Fallback to a safe default normal
    }

    return normalize(result);
}

float CalculateShadowCascade(FragmentIn input, DirectionalLight Light)
{
    // Accessing the cascade buffer from the descriptor heap.
    ConstantBuffer<CascadeBuffer> cascades = ResourceDescriptorHeap[PushConstants.CascadeBufferIndex];

    // Ensure depth value is positive and consistent.
    float depthValue = abs(input.FragPosView.z);

    // Determine the cascade layer based on depth.
    int layer = 0;
    // for (int i = 0; i < SHADOW_CASCADE_COUNT; i++) {
    //     if (depthValue < cascades.Cascades[i].Split) {
    //         layer = i;
    //         break;
    //     }
    // }
    // if (layer == -1) {
    //     layer = SHADOW_CASCADE_COUNT - 1; // Use the last cascade if no match.
    // }

    // Ensure proper sampler and shadow map access.
    SamplerState sampler = SamplerDescriptorHeap[PushConstants.SamplerIndex];
    Texture2D<float> shadowMap = ResourceDescriptorHeap[cascades.Cascades[layer].SRVIndex];

    // Transform world position to light's view space and then to light's clip space.
    float4 lightViewPosition = mul(cascades.Cascades[layer].View, float4(input.FragPosWorld, 1.0));
    float4 fragPosLightSpace = mul(cascades.Cascades[layer].Proj, lightViewPosition);

    // Handle perspective division and normalize to [0, 1] space.
    float3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords.xy = projCoords.xy * 0.5 + 0.5;
    projCoords.y = 1.0 - projCoords.y;

    // Avoid accessing out-of-bounds texels.
    if (projCoords.x < 0.0 || projCoords.x > 1.0 || projCoords.y < 0.0 || projCoords.y > 1.0) {
        return 1.0;
    }

    float currentDepth = projCoords.z;

    // Calculate the shadow bias dynamically based on surface normal and light direction.
    float3 N = GetNormal(input);
    float bias = max(0.05 * (1.0 - dot(N, Light.Direction)), 0.005);

    // PCF sampling for soft shadows.
    uint shadowWidth, shadowHeight;
    shadowMap.GetDimensions(shadowWidth, shadowHeight);

    float shadow = AMBIENT;
    float2 texelSize = 1.0 / float2(shadowWidth, shadowHeight);
    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            float2 offset = float2(x, y) * texelSize;
            float pcfDepth = shadowMap.Sample(sampler, projCoords.xy + offset).r;
            shadow += (currentDepth - bias) > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0; // Average over the 3x3 kernel.

    return 0.0;
}

float3 CalculatePoint(PointLight Light, FragmentIn Input, float3 Albedo)
{
    ConstantBuffer<Camera> Cam = ResourceDescriptorHeap[PushConstants.CameraIndex];

    float3 N = GetNormal(Input);
    float Distance = length(Light.Position - Input.FragPosWorld);
    float Attenuation = 1.0 / (Distance * Distance);

    if (Attenuation > 0.0) {
        float3 LightDirection = normalize(Light.Position - Input.FragPosWorld);
        float NdotL = max(dot(N, LightDirection) * Light.Radius, AMBIENT);
        return (NdotL * Albedo * Attenuation * Light.Color.xyz);
    } else {
        return Albedo * AMBIENT;
    }
}

float3 CalculateSun(DirectionalLight Light, FragmentIn Input, float3 Albedo)
{
    ConstantBuffer<Camera> Cam = ResourceDescriptorHeap[PushConstants.CameraIndex];

    float3 N = GetNormal(Input);
    if (any(isnan(N))) {
        return float3(1.0, 0.0, 1.0); // Return a magenta color for NaN debugging
    }

    float attenuation = clamp(dot(N, -Light.Direction), 0.0, 1.0);
    if (attenuation > 0.0f) {
        float NdotL = max(dot(N, -Light.Direction), AMBIENT);
        return (NdotL * Albedo * Light.Strength * Light.Color.xyz);
    } else {
        return Albedo * AMBIENT;
    }
}

float TraceShadow(DirectionalLight Light, FragmentIn Input)
{
    RaytracingAccelerationStructure TLAS = ResourceDescriptorHeap[PushConstants.AccelStructure];

    float3 N = GetNormal(Input);
    float attenuation = clamp(dot(N, -Light.Direction), 0.0, 1.0);
    if (true) {
        RayDesc desc;
        desc.Origin = Input.FragPosWorld.xyz;
        desc.Direction = -Light.Direction;
        desc.TMin = 0.01;
        desc.TMax = 5000.0;

        RayQuery<RAY_FLAG_CULL_NON_OPAQUE | RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH | RAY_FLAG_SKIP_PROCEDURAL_PRIMITIVES> q;
        q.TraceRayInline(TLAS, RAY_FLAG_CULL_NON_OPAQUE | RAY_FLAG_SKIP_PROCEDURAL_PRIMITIVES | RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH, 0xFF, desc);
        q.Proceed();

        if (q.CommittedStatus() == COMMITTED_TRIANGLE_HIT) {
            return AMBIENT;
        } else {
            return 1.0;
        }
    }

    return 1.0f;
}

float4 PSMain(FragmentIn Input) : SV_Target
{
    ConstantBuffer<CascadeBuffer> CascadeInfo = ResourceDescriptorHeap[PushConstants.CascadeBufferIndex];
    ConstantBuffer<LightData> Lights = ResourceDescriptorHeap[PushConstants.LightIndex];
    Texture2D Albedo = ResourceDescriptorHeap[PushConstants.TextureIndex];
    SamplerState Sampler = SamplerDescriptorHeap[PushConstants.SamplerIndex];

    float4 Color = Albedo.Sample(Sampler, Input.UV);    
    float3 Lo = Color.xyz * AMBIENT;
    
    float shadow = CalculateShadowCascade(Input, Lights.Sun);
    Lo += CalculateSun(Lights.Sun, Input, Color.xyz) * shadow;
    
    for (int i = 0; i < Lights.LightCount; i++) {
        Lo += CalculatePoint(Lights.Lights[i], Input, Color.xyz);
    }
    return float4(Lo, 1.0);
}
