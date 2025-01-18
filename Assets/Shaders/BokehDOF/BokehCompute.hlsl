//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2025-01-18 15:13:37
//

static const float2 BOKEH_CIRCLE[] = {
	2.0f * float2(1.000000f, 0.000000f),
	2.0f * float2(0.707107f, 0.707107f),
	2.0f * float2(-0.000000f, 1.000000f),
	2.0f * float2(-0.707107f, 0.707107f),
	2.0f * float2(-1.000000f, -0.000000f),
	2.0f * float2(-0.707106f, -0.707107f),
	2.0f * float2(0.000000f, -1.000000f),
	2.0f * float2(0.707107f, -0.707107f),
	
	4.0f * float2(1.000000f, 0.000000f),
	4.0f * float2(0.923880f, 0.382683f),
	4.0f * float2(0.707107f, 0.707107f),
	4.0f * float2(0.382683f, 0.923880f),
	4.0f * float2(-0.000000f, 1.000000f),
	4.0f * float2(-0.382684f, 0.923879f),
	4.0f * float2(-0.707107f, 0.707107f),
	4.0f * float2(-0.923880f, 0.382683f),
	4.0f * float2(-1.000000f, -0.000000f),
	4.0f * float2(-0.923879f, -0.382684f),
	4.0f * float2(-0.707106f, -0.707107f),
	4.0f * float2(-0.382683f, -0.923880f),
	4.0f * float2(0.000000f, -1.000000f),
	4.0f * float2(0.382684f, -0.923879f),
	4.0f * float2(0.707107f, -0.707107f),
	4.0f * float2(0.923880f, -0.382683f),

	6.0f * float2(1.000000f, 0.000000f),
	6.0f * float2(0.965926f, 0.258819f),
	6.0f * float2(0.866025f, 0.500000f),
	6.0f * float2(0.707107f, 0.707107f),
	6.0f * float2(0.500000f, 0.866026f),
	6.0f * float2(0.258819f, 0.965926f),
	6.0f * float2(-0.000000f, 1.000000f),
	6.0f * float2(-0.258819f, 0.965926f),
	6.0f * float2(-0.500000f, 0.866025f),
	6.0f * float2(-0.707107f, 0.707107f),
	6.0f * float2(-0.866026f, 0.500000f),
	6.0f * float2(-0.965926f, 0.258819f),
	6.0f * float2(-1.000000f, -0.000000f),
	6.0f * float2(-0.965926f, -0.258820f),
	6.0f * float2(-0.866025f, -0.500000f),
	6.0f * float2(-0.707106f, -0.707107f),
	6.0f * float2(-0.499999f, -0.866026f),
	6.0f * float2(-0.258819f, -0.965926f),
	6.0f * float2(0.000000f, -1.000000f),
	6.0f * float2(0.258819f, -0.965926f),
	6.0f * float2(0.500000f, -0.866025f),
	6.0f * float2(0.707107f, -0.707107f),
	6.0f * float2(0.866026f, -0.499999f),
	6.0f * float2(0.965926f, -0.258818f),
};

struct Data
{
    int LinearSampler;
    int PointSampler;
    int DOFNear;
    int DOFFar;

    int COCTexture;
    int COCNearTexture;
    int ColorTexture;
    int ColorMulFar;

    float KernelSize;
    float3 Pad;
};

ConstantBuffer<Data> Constants : register(b0);

float4 Near(float kernelScale, float2 pixelSize, float2 UV)
{
    SamplerState linearSampler = SamplerDescriptorHeap[Constants.LinearSampler];
    SamplerState pointSampler = SamplerDescriptorHeap[Constants.PointSampler];
    Texture2D<float4> colorTexture = ResourceDescriptorHeap[Constants.ColorTexture];

    float4 result = colorTexture.Sample(pointSampler, UV);
    for (int i = 0; i < 48; i++) {
        float2 offset = kernelScale * BOKEH_CIRCLE[i] * pixelSize;
        result += colorTexture.Sample(linearSampler, UV + offset);
    }
    return result / 49.0f;
}

float4 Far(float kernelScale, float2 pixelSize, float2 UV)
{
    SamplerState linearSampler = SamplerDescriptorHeap[Constants.LinearSampler];
    SamplerState pointSampler = SamplerDescriptorHeap[Constants.PointSampler];
    Texture2D<float2> cocTexture = ResourceDescriptorHeap[Constants.COCTexture];
    Texture2D<float4> colorFarMulTexture = ResourceDescriptorHeap[Constants.ColorMulFar];

    float4 result = colorFarMulTexture.Sample(pointSampler, UV);
	float weightsSum = cocTexture.Sample(pointSampler, UV).y;
	
	for (int i = 0; i < 48; i++) {
		float2 offset = kernelScale * BOKEH_CIRCLE[i] * pixelSize;
		
		float cocSample = cocTexture.SampleLevel(linearSampler, UV + offset, 0).y;
		float4 sample = colorFarMulTexture.SampleLevel(linearSampler, UV + offset, 0);
		
		result += sample;
		weightsSum += cocSample;
	}
	return result / weightsSum;	
}

[numthreads(8, 8, 1)]
void CSMain(uint3 TID : SV_DispatchThreadID)
{
    SamplerState pointSampler = SamplerDescriptorHeap[Constants.PointSampler];
    Texture2D<float4> colorTexture = ResourceDescriptorHeap[Constants.ColorTexture];
    Texture2D<float2> cocTexture = ResourceDescriptorHeap[Constants.COCTexture];
    Texture2D<float> cocNearTexture = ResourceDescriptorHeap[Constants.COCNearTexture];
    RWTexture2D<float4> dofNear = ResourceDescriptorHeap[Constants.DOFNear];
    RWTexture2D<float4> dofFar = ResourceDescriptorHeap[Constants.DOFFar];

    //
    uint width, height;
    cocNearTexture.GetDimensions(width, height);
    float2 pixelSize = 1.0 / float2(width, height);
    float2 UV = TID.xy / float2(width, height);
    if (TID.x > width || TID.y > height)
        return;

    //
    float cocNearBlurred = cocNearTexture.SampleLevel(pointSampler, UV, 0).x;
	float cocFar = cocTexture.SampleLevel(pointSampler, UV, 0).y;
	float4 color = colorTexture.SampleLevel(pointSampler, UV, 0);

    float4 outputNear = 0.0;
    float4 outputFar = 0.0;

    if (cocNearBlurred > 0.0f)
		outputNear = Near(Constants.KernelSize, pixelSize, UV);
	else
		outputNear = color;
	if (cocFar > 0.0f)
		outputFar = Far(Constants.KernelSize, pixelSize, UV);
	else
		outputFar = 0.0f;
    
    //
    dofNear[TID.xy] = outputNear;
    dofFar[TID.xy] = outputFar;
}
