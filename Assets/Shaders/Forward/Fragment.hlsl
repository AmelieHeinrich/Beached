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

    // Samplers
    int SamplerIndex;
    int ShadowSamplerIndex;

    // Acceleration structures
    int AccelStructure;
};

ConstantBuffer<Settings> PushConstants : register(b0);

float CalculateShadowCascade(FragmentIn input, uint cascadeIndex)
{
    ConstantBuffer<CascadeBuffer> CascadeInfo = ResourceDescriptorHeap[PushConstants.CascadeBufferIndex];
    ConstantBuffer<LightData> Lights = ResourceDescriptorHeap[PushConstants.LightIndex];
    Texture2D ShadowMap = ResourceDescriptorHeap[CascadeInfo.Cascades[cascadeIndex].SRVIndex];
    SamplerState ShadowSampler = SamplerDescriptorHeap[PushConstants.ShadowSamplerIndex];

    float4 lightPos = mul(CascadeInfo.Cascades[cascadeIndex].ViewProj, float4(input.FragPosWorld, 1.0));

    float3 projectionCoords = lightPos.xyz / lightPos.w;
    projectionCoords.xy = projectionCoords.xy * 0.5 + 0.5;
    projectionCoords.y = 1.0 - projectionCoords.y;

    float closestDepth = ShadowMap.Sample(ShadowSampler, projectionCoords.xy, 0).r;
    float currentDepth = projectionCoords.z;

    float bias = max(0.05 * (1.0 - dot(input.Normal, Lights.Sun.Direction.xyz)), 0.005);
	
    float shadowWidth, shadowHeight;
    ShadowMap.GetDimensions(shadowWidth, shadowHeight);

    float shadow = 1.0;
    float2 texelSize = 1.0 / float2(shadowWidth, shadowHeight);

    float dist = ShadowMap.Sample(ShadowSampler, projectionCoords.xy).r;
	if (lightPos.w > 0 && dist < projectionCoords.z - bias) {
		shadow = 0.3;
	}

    if (projectionCoords.z > 1.0)
        shadow = 0.0;
    return shadow;
}

float3 CalculatePoint(PointLight Light, FragmentIn Input, float3 Albedo)
{
    ConstantBuffer<Camera> Cam = ResourceDescriptorHeap[PushConstants.CameraIndex];

    // TODO: Normal maps.
    float Distance = length(Light.Position - Input.FragPosWorld);
    float Attenuation = 1.0 / (Distance * Distance);

    if (Attenuation > 0.0) {
        float3 LightDirection = normalize(Light.Position - Input.FragPosWorld);
        float NdotL = max(dot(Input.Normal, LightDirection) * Light.Radius, AMBIENT);
        return (NdotL * Albedo * Attenuation * Light.Color.xyz);
    } else {
        return Albedo * AMBIENT;
    }
}

float3 CalculateSun(DirectionalLight Light, FragmentIn Input, float3 Albedo)
{
    ConstantBuffer<Camera> Cam = ResourceDescriptorHeap[PushConstants.CameraIndex];

    // TODO: Normal maps.
    float attenuation = clamp(dot(Input.Normal, -Light.Direction), 0.0, 1.0);
    if (attenuation > 0.0f) {
        float NdotL = max(dot(Input.Normal, -Light.Direction), AMBIENT);
        return (NdotL * Albedo * Light.Strength * Light.Color.xyz);
    } else {
        return Albedo * AMBIENT;
    }
}

float TraceShadow(DirectionalLight Light, FragmentIn Input)
{
    RaytracingAccelerationStructure TLAS = ResourceDescriptorHeap[PushConstants.AccelStructure];

    float attenuation = clamp(dot(Input.Normal, -Light.Direction), 0.0, 1.0);
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

    // uint cascadeIndex = 0;
	// for (uint i = 0; i < SHADOW_CASCADE_COUNT - 1; ++i) {
	// 	if(Input.FragPosView.z < CascadeInfo.Cascades[i].Split) {
	// 		cascadeIndex = i + 1;
	// 	}
	// }

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
