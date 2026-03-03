module ScreenQuadVertexShader;

struct VSInput
{
    float3 position : POSITION;
    float2 texCoord : TEXCOORD0;
};

struct VSOutput
{
    float4 position : SV_Position;
    float2 uv       : TEXCOORD0;
};

[shader("vertex")]
VSOutput Main(VSInput input)
{
    VSOutput output;
    output.position = float4(input.position, 1.0f);
    output.uv = input.texCoord;
    output.uv.y = 1.0f - output.uv.y;
    return output;
}
