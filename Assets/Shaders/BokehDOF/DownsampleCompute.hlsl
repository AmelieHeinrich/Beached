//
// > Notice: Amélie Heinrich @ 2024
// > Create Time: 2025-01-18 13:00:09
//

struct Data
{
    int PointClamp;
    int LinearClamp;
    int ColorTexture;
    int COCTexture;

    int ColorX4;
    int COCMulFarX4;
    int COCX4;
    int Pad;
};

ConstantBuffer<Data> Constants : register(b0);

[numthreads(8, 8, 1)]
void CSMain(uint3 TID : SV_DispatchThreadID)
{
    // Resources
    SamplerState pointSampler = SamplerDescriptorHeap[Constants.PointClamp];
    SamplerState linearSampler = SamplerDescriptorHeap[Constants.LinearClamp];
    Texture2D<float4> colorTexture = ResourceDescriptorHeap[Constants.ColorTexture];
    Texture2D<float2> cocTexture = ResourceDescriptorHeap[Constants.COCTexture];
    RWTexture2D<float4> colorTextureX4 = ResourceDescriptorHeap[Constants.ColorX4];
    RWTexture2D<float4> colorTextureMulFarX4 = ResourceDescriptorHeap[Constants.COCMulFarX4];
    RWTexture2D<float2> cocTextureX4 = ResourceDescriptorHeap[Constants.COCX4];

    // Shader
    uint quarterWidth, quarterHeight;
    colorTextureX4.GetDimensions(quarterWidth, quarterHeight);

    //
    float2 pixelSize = 1.0f / float2(quarterWidth, quarterHeight);
    float2 UV = TID.xy / float2(quarterWidth, quarterHeight);
    float2 texCoord00 = UV + float2(-0.25f, -0.25f) * pixelSize;
	float2 texCoord10 = UV + float2( 0.25f, -0.25f) * pixelSize;
	float2 texCoord01 = UV + float2(-0.25f,  0.25f) * pixelSize;
	float2 texCoord11 = UV + float2( 0.25f,  0.25f) * pixelSize;
    if (TID.x > quarterWidth || TID.y > quarterHeight)
        return;

    //
    float4 color = colorTexture.SampleLevel(linearSampler, UV, 0);
	float2 coc = cocTexture.SampleLevel(pointSampler, texCoord00, 0);

    float cocFar00 = cocTexture.SampleLevel(pointSampler, texCoord00, 0).y;
	float cocFar10 = cocTexture.SampleLevel(pointSampler, texCoord10, 0).y;
	float cocFar01 = cocTexture.SampleLevel(pointSampler, texCoord01, 0).y;
	float cocFar11 = cocTexture.SampleLevel(pointSampler, texCoord11, 0).y;

    float weight00 = 1000.0f;
	float4 colorMulCOCFar = weight00 * colorTexture.SampleLevel(pointSampler, texCoord00, 0);
	float weightsSum = weight00;
	
	float weight10 = 1.0f / (abs(cocFar00 - cocFar10) + 0.001f);
	colorMulCOCFar += weight10 * colorTexture.SampleLevel(pointSampler, texCoord10, 0);
	weightsSum += weight10;
	
	float weight01 = 1.0f / (abs(cocFar00 - cocFar01) + 0.001f);
	colorMulCOCFar += weight01 * colorTexture.SampleLevel(pointSampler, texCoord01, 0);
	weightsSum += weight01;
	
	float weight11 = 1.0f / (abs(cocFar00 - cocFar11) + 0.001f);
	colorMulCOCFar += weight11 * colorTexture.SampleLevel(pointSampler, texCoord11, 0);
	weightsSum += weight11;

	colorMulCOCFar /= weightsSum;
	colorMulCOCFar *= coc.y;

    //

    colorTextureX4[TID.xy] = color;
    colorTextureMulFarX4[TID.xy] = colorMulCOCFar;
    cocTextureX4[TID.xy] = coc;
}
