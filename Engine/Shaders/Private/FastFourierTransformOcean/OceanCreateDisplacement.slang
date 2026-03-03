module OceanCreateDisplacement;

static const float PI = 3.14159f;
static const float g = 9.80665f;

struct FOceanCreateDisplacementParameters
{
    RWTexture2D<float2> HeightSpectrumRT;
    RWTexture2D<float2> DisplaceXSpectrumRT;
    RWTexture2D<float2> DisplaceYSpectrumRT;
    RWTexture2D<float4> DisplaceRT;
}
ParameterBlock<FOceanCreateDisplacementParameters> Params;

void GenerationDisplace(uint3 id)
{
    float z = length(Params.HeightSpectrumRT[id.xy]);     //高度
    float x = length(Params.DisplaceXSpectrumRT[id.xy]);  //x轴偏移
    float y = length(Params.DisplaceYSpectrumRT[id.xy]);  //y轴偏移

    Params.DisplaceRT[id.xy] = float4(x, y, z, 0);
}

[shader("compute")]
[numthreads(32, 32, 1)]
void Main(uint3 threadID : SV_DispatchThreadID)
{
    GenerationDisplace(threadID);
}
