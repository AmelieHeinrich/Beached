//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-03 18:52:15
//

#include "Assets/Shaders/Lights.hlsl"
#include "Assets/Shaders/Camera.hlsl"
#include "Assets/Shaders/Cascade.hlsl"
#include "Assets/Shaders/Shadow.hlsl"

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
    float4 FragPosWorld : POSITION;
    float4 FragPosView : POSITION1;
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

float CalculateShadowCascade(FragmentIn input, DirectionalLight Light, int layer)
{
    ConstantBuffer<CascadeBuffer> cascades = ResourceDescriptorHeap[PushConstants.CascadeBufferIndex];
    
    Cascade cascade = cascades.Cascades[layer];
    SamplerComparisonState sampler = SamplerDescriptorHeap[PushConstants.ShadowSamplerIndex];
    Texture2D<float> shadowMap = ResourceDescriptorHeap[NonUniformResourceIndex(cascades.Cascades[layer].SRVIndex)];
    float3 N = GetNormal(input);

    float baseBias = 0.005;
    if (layer == SHADOW_CASCADE_COUNT) {
        baseBias *= 1 / (CAMERA_FAR * 0.5);
    } else {
        baseBias *= 1 / (cascade.Split * 0.5);
    }

    // Calculate dynamic slope-scaled bias
    float3 lightDir = normalize(Light.Direction);
    float3 fragToLight = normalize(lightDir);
    float NdotL = abs(dot(N, fragToLight));

    // Apply slope-scaled bias
    float slopeBias = saturate(1.0 - NdotL) * 0.01; // Tweak the scale factor (0.01) as needed.
    float finalBias = baseBias + slopeBias;

    int kernelSize = SHADOW_PCF_KERNELS[layer];
    return ComputePCF(shadowMap,
                      sampler,
                      input.FragPosWorld,
                      Light.Direction,
                      cascade.View,
                      cascade.Proj,
                      finalBias,
                      kernelSize);
}

float3 CalculatePoint(PointLight Light, FragmentIn Input, float3 Albedo)
{
    ConstantBuffer<Camera> Cam = ResourceDescriptorHeap[PushConstants.CameraIndex];

    float3 N = GetNormal(Input);
    float Distance = length(Light.Position - Input.FragPosWorld.xyz);
    float Attenuation = 1.0 / (Distance * Distance);

    if (Attenuation > 0.0) {
        float3 LightDirection = normalize(Light.Position - Input.FragPosWorld.xyz);
        float NdotL = max(dot(N, LightDirection) * Light.Radius, AMBIENT);
        return (NdotL * Albedo * Attenuation * Light.Color.xyz);
    } else {
        return Albedo * AMBIENT;
    }
}

float3 CalculateSun(DirectionalLight Light, FragmentIn Input, float3 Albedo, int layer)
{
    ConstantBuffer<Camera> Cam = ResourceDescriptorHeap[PushConstants.CameraIndex];

    float shadow = CalculateShadowCascade(Input, Light, layer);

    float3 N = GetNormal(Input);
    float attenuation = clamp(dot(N, -Light.Direction), 0.0, 1.0);
    if (attenuation > 0.0f) {
        float NdotL = max(dot(N, -Light.Direction), AMBIENT);
        return (NdotL * Albedo * Light.Strength * Light.Color.xyz) * shadow;
    } else {
        return Albedo * AMBIENT * shadow;
    }
}

float4 PSMain(FragmentIn Input) : SV_Target
{    
    // Get cascade layer
    ConstantBuffer<CascadeBuffer> CascadeInfo = ResourceDescriptorHeap[PushConstants.CascadeBufferIndex];
    int layer = -1;
    for (int i = 0; i < SHADOW_CASCADE_COUNT; i++) {
        if (abs(Input.FragPosView.z) < CascadeInfo.Cascades[i].Split) {
            layer = i;
            break;
        }
    }
    if (layer == -1) {
        layer = SHADOW_CASCADE_COUNT - 1;
    }

    // Get texture data
    Texture2D Albedo = ResourceDescriptorHeap[PushConstants.TextureIndex];
    SamplerState Sampler = SamplerDescriptorHeap[PushConstants.SamplerIndex];
    float4 Color = Albedo.Sample(Sampler, Input.UV);
    
    // Get light data
    ConstantBuffer<LightData> Lights = ResourceDescriptorHeap[PushConstants.LightIndex];
    StructuredBuffer<PointLight> PointLights = ResourceDescriptorHeap[Lights.PointLightBuffer];
    
    float3 Lo = Color.xyz * AMBIENT;
    if (Lights.UseSun)
        Lo += CalculateSun(Lights.Sun, Input, Color.xyz, layer);
    for (int i = 0; i < Lights.PointLightCount; i++) {
        Lo += CalculatePoint(PointLights[i], Input, Color.xyz);
    }
    return float4(Lo, 1.0);
}
