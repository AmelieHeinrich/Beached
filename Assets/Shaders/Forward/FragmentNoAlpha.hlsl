//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2024-12-03 18:52:15
//

#include "Assets/Shaders/Lights.hlsl"
#include "Assets/Shaders/Camera.hlsl"
#include "Assets/Shaders/Cascade.hlsl"
#include "Assets/Shaders/Shadow.hlsl"

static const float AMBIENT = 0.01;

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
    int ClampSamplerIndex;
    int ShadowSamplerIndex;

    // Acceleration structures
    int AccelStructure;
};

struct Model
{
    column_major float4x4 Transform;
    column_major float4x4 InvTransform;
    float4 MaterialColor;
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

    float bias = max(0.05 * (1.0 - dot(N, Light.Direction)), 0.005);
    if (layer == SHADOW_CASCADE_COUNT) {
        bias *= 1 / (CAMERA_FAR * 0.5);
    } else {
        bias *= 1 / (cascade.Split * 0.5);
    }

    int kernelSize = SHADOW_PCF_KERNELS[layer];
    return PCFCascade(shadowMap,
                      sampler,
                      input.FragPosWorld,
                      Light.Direction,
                      cascade.View,
                      cascade.Proj,
                      bias,
                      kernelSize);
}

float CalculateShadowPoint(FragmentIn input, PointLight light)
{
    ConstantBuffer<Camera> Cam = ResourceDescriptorHeap[PushConstants.CameraIndex];
    SamplerState sampler = SamplerDescriptorHeap[PushConstants.ClampSamplerIndex];
    TextureCube<float> shadowMap = ResourceDescriptorHeap[light.ShadowCubemap];
    float3 N = GetNormal(input);
    
    return PCFPoint(shadowMap,
                    sampler,
                    input.FragPosWorld,
                    Cam.Position,
                    light.Position,
                    1);
}

float CalculateShadowSpot(FragmentIn input, SpotLight light)
{
    ConstantBuffer<Camera> Cam = ResourceDescriptorHeap[PushConstants.CameraIndex];
    SamplerComparisonState sampler = SamplerDescriptorHeap[PushConstants.ShadowSamplerIndex];
    Texture2D<float> shadowMap = ResourceDescriptorHeap[light.ShadowMap];
    float3 N = GetNormal(input);
    
    return PCFSpot(shadowMap,
                   sampler,
                   input.FragPosWorld,
                   light.LightView,
                   light.LightProj);
}

float3 CalculatePoint(PointLight Light, FragmentIn Input, float3 Albedo)
{
    ConstantBuffer<Camera> Cam = ResourceDescriptorHeap[PushConstants.CameraIndex];

    float shadow = Light.CastShadows ? CalculateShadowPoint(Input, Light) : 1.0;

    float3 N = GetNormal(Input);
    float Distance = length(Light.Position - Input.FragPosWorld.xyz);
    float Attenuation = 1.0 / (Distance * Distance);

    if (Attenuation > 0.0) {
        float3 LightDirection = normalize(Light.Position - Input.FragPosWorld.xyz);
        float NdotL = max(dot(N, LightDirection) * Light.Radius, AMBIENT);
        return (NdotL * Albedo * Attenuation * Light.Color.xyz) * shadow;
    } else {
        return Albedo * AMBIENT;
    }
}

float3 CalculateSpot(SpotLight light, FragmentIn input, float3 Albedo)
{
    ConstantBuffer<Camera> Cam = ResourceDescriptorHeap[PushConstants.CameraIndex];

    // Calculate surface normal and light direction
    float3 N = GetNormal(input);
    float3 L = normalize(light.Position - input.FragPosWorld.xyz);
    float distance = length(light.Position - input.FragPosWorld.xyz);

    // Shadow calculation
    float shadow = CalculateShadowSpot(input, light);
    float theta = dot(L, normalize(-light.Direction));
    float smoothFactor = smoothstep(cos(light.OuterRadius), cos(light.Radius), theta);
    float intensity = smoothFactor;
    if (theta > cos(light.OuterRadius)) {
        float NdotL = max(dot(N, L), AMBIENT);
        return (NdotL * Albedo * intensity * light.Color.xyz) * shadow;
    } else {
        // Return ambient lighting outside the spotlight
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

struct FragOutput
{
    float4 Target : SV_Target;
};

FragOutput PSMain(FragmentIn Input)
{    
    // Get cascade layer
    ConstantBuffer<CascadeBuffer> CascadeInfo = ResourceDescriptorHeap[PushConstants.CascadeBufferIndex];
    ConstantBuffer<Model> Instance = ResourceDescriptorHeap[PushConstants.ModelIndex];

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
    float4 Color = Albedo.Sample(Sampler, Input.UV) * Instance.MaterialColor;
    
    // Get light data
    ConstantBuffer<LightData> Lights = ResourceDescriptorHeap[PushConstants.LightIndex];
    StructuredBuffer<PointLight> PointLights = ResourceDescriptorHeap[Lights.PointLightSRV];
    StructuredBuffer<SpotLight> SpotLights = ResourceDescriptorHeap[Lights.SpotLightSRV];
    
    float3 Lo = Color.xyz * AMBIENT;
    if (Lights.UseSun) {
        Lo += CalculateSun(Lights.Sun, Input, Color.xyz, layer) * 0.5;
    }
    for (int i = 0; i < Lights.PointLightCount; i++) {
        Lo += CalculatePoint(PointLights[i], Input, Color.xyz);
    }
    for (int i = 0; i < Lights.SpotLightCount; i++) {
        Lo += CalculateSpot(SpotLights[i], Input, Color.xyz);
    }

    FragOutput output;
    output.Target = float4(Lo, 1.0);
    return output;
}
