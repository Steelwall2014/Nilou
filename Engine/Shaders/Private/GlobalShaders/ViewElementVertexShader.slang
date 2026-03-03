module ViewElementVertexShader;

struct FViewParameters
{
    float4x4 RelWorldToClip;
    float3   CameraPosition;
};
ParameterBlock<FViewParameters> Params;

struct VSInput
{
    float3 position : POSITION;
    float3 color    : COLOR;
};

struct VSOutput
{
    float4 position : SV_Position;
    float3 color    : COLOR;
};

[shader("vertex")]
VSOutput Main(VSInput input)
{
    VSOutput output;
    output.position = mul(Params.RelWorldToClip, float4(input.position - Params.CameraPosition, 1.0f));
    output.color = input.color;
    return output;
}
