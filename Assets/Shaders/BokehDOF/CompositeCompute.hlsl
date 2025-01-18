//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2025-01-18 15:50:27
//

struct Data
{
    int PointSampler;
    int LinearSampler;
    int ColorTexture;
    int COCTexture;

    int COCTextureX4;
    int COCNear;
    int DOFNear;
    int DOFFar;

    float Strength;
    float3 Pad;
};

ConstantBuffer<Data> Constants : register(b0);

[numthreads(8, 8, 1)]
void CSMain(uint3 TID : SV_DispatchThreadID)
{
    // So many resources I HATE BINDLESS I NEED TO MAKE MACROS
    SamplerState pointSampler = SamplerDescriptorHeap[Constants.PointSampler];
    SamplerState linearSampler = SamplerDescriptorHeap[Constants.LinearSampler];
    RWTexture2D<float4> colorTexture = ResourceDescriptorHeap[Constants.ColorTexture];
    Texture2D<float2> cocTexture = ResourceDescriptorHeap[Constants.COCTexture];
    Texture2D<float2> cocTextureX4 = ResourceDescriptorHeap[Constants.COCTextureX4];
    Texture2D<float> cocNearTexture = ResourceDescriptorHeap[Constants.COCNear];
    Texture2D<float4> nearTexture = ResourceDescriptorHeap[Constants.DOFNear];
    Texture2D<float4> farTexture = ResourceDescriptorHeap[Constants.DOFFar];

    //
    uint width, height;
    colorTexture.GetDimensions(width, height);
    if (TID.x > width || TID.y > height)
        return;
    float2 pixelSize = 1.0 / float2(width, height);
    float2 UV = TID.xy * pixelSize;

    //
    float4 result = colorTexture[TID.xy];

    // Far field
    {
        float2 texCoord00 = UV;
		float2 texCoord10 = UV + float2(pixelSize.x, 0.0f);
		float2 texCoord01 = UV + float2(0.0f, pixelSize.y);
		float2 texCoord11 = UV + float2(pixelSize.x, pixelSize.y);

		float cocFar = cocTexture.Sample(pointSampler, UV).y;
		float4 cocsFar_x4 = cocTextureX4.GatherGreen(pointSampler, texCoord00).wzxy;
		float4 cocsFarDiffs = abs(cocFar.xxxx - cocsFar_x4);

		float4 dofFar00 = farTexture.Sample(pointSampler, texCoord00);
		float4 dofFar10 = farTexture.Sample(pointSampler, texCoord10);
		float4 dofFar01 = farTexture.Sample(pointSampler, texCoord01);
		float4 dofFar11 = farTexture.Sample(pointSampler, texCoord11);

		float2 imageCoord = UV / pixelSize;
		float2 fractional = frac(imageCoord);
		float a = (1.0f - fractional.x) * (1.0f - fractional.y);
		float b = fractional.x * (1.0f - fractional.y);
		float c = (1.0f - fractional.x) * fractional.y;
		float d = fractional.x * fractional.y;

		float4 dofFar = 0.0f;
		float weightsSum = 0.0f;

		float weight00 = a / (cocsFarDiffs.x + 0.001f);
		dofFar += weight00 * dofFar00;
		weightsSum += weight00;

		float weight10 = b / (cocsFarDiffs.y + 0.001f);
		dofFar += weight10 * dofFar10;
		weightsSum += weight10;

		float weight01 = c / (cocsFarDiffs.z + 0.001f);
		dofFar += weight01 * dofFar01;
		weightsSum += weight01;

		float weight11 = d / (cocsFarDiffs.w + 0.001f);
		dofFar += weight11 * dofFar11;
		weightsSum += weight11;

		dofFar /= weightsSum;

		result = lerp(result, dofFar, Constants.Strength * cocFar);
    }

    // Near field
    {
        float cocNear = cocNearTexture.Sample(linearSampler, UV).x;
		float4 dofNear = nearTexture.Sample(linearSampler, UV);

		result = lerp(result, dofNear, Constants.Strength * cocNear);
    }

    //
    colorTexture[TID.xy] = result;
}
