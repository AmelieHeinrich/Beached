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

    // Epsilon because the normal can sometimes be a bit *freaky* when crossing that vector
    float3 T = Q1 * ST2.y - Q2 * ST1.y + 0.01;
    float3 B = cross(normal, T) + 0.01;
    float3 N = normal + 0.01;
    float3x3 TBN = float3x3(normalize(T), normalize(B), N);

    return normalize(mul(tangentNormal, TBN));
}

float CalculateShadowCascade(FragmentIn input, uint cascadeIndex)
{
    return 1.0;
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
    if (Color.a < 0.1)
        discard;
    
    float3 Lo = Color.xyz * AMBIENT;
    
    float shadow = 1.0;
    Lo += CalculateSun(Lights.Sun, Input, Color.xyz) * shadow;
    
    for (int i = 0; i < Lights.LightCount; i++) {
        Lo += CalculatePoint(Lights.Lights[i], Input, Color.xyz);
    }
    return float4(Lo, 1.0);
}
