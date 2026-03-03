module RasterGenerateMips;

struct RasterGenerateMipsVertexInput
{
    float4 InPosition : POSITION;
    float2 InUV : TEXCOORD0;
};

struct RasterGenerateMipsVertexOutput
{
    float2 UV : TEXCOORD0;
    float4 Position : SV_Position;
};

[shader("vertex")]
RasterGenerateMipsVertexOutput MainVS(RasterGenerateMipsVertexInput input)
{
    RasterGenerateMipsVertexOutput output;
    output.Position = input.InPosition;
    output.UV = input.InUV;
    return output;
}

struct FRasterGenerateMipsParameters
{
    Sampler2D MipInSRV;
    float2 HalfTexelSize;
    float Level;
};
ParameterBlock<FRasterGenerateMipsParameters> Params;

[shader("pixel")]
float4 MainPS(RasterGenerateMipsVertexOutput input)
{
    return Params.MipInSRV.SampleLevel(input.UV + Params.HalfTexelSize, Params.Level);
}
