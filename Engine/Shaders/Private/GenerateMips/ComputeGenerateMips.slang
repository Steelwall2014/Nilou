module ComputeGenerateMips;

struct FComputeGenerateMipsInputParameters
{
	Sampler2D MipInSRV;
}
struct FComputeGenerateMipsOutputParameters
{
	RWTexture2D<float4> MipOutUAV;
}
ParameterBlock<FComputeGenerateMipsInputParameters> InputParams;
ParameterBlock<FComputeGenerateMipsOutputParameters> OutputParams;

[shader("compute")]
[numthreads(16, 16, 1)]
void Main(uint3 threadID : SV_DispatchThreadID)
{
	uint width, height;
	OutputParams.MipOutUAV.GetDimensions(width, height);
	
	uint px = threadID.x;
	uint py = threadID.y;
	if (px >= width || py >= height)
	{
		return;
	}

	float2 uv = (float2(px, py) + float2(0.5)) / float2(width, height);
	float4 Out = InputParams.MipInSRV.SampleLevelZero(uv);
	OutputParams.MipOutUAV[uint2(px, py)] = Out;
}