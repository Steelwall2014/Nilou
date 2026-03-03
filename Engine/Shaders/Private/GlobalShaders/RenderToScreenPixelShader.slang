module RenderToScreenPixelShader;

struct FRenderToScreenParameters
{
    float GammaCorrection;
    int   bEnableToneMapping;
    Texture2D<float4> SceneColor;
    SamplerState      SceneColorSampler;
};
ParameterBlock<FRenderToScreenParameters> Params;

struct PSInput
{
    float2 uv : TEXCOORD0;
};

struct PSOutput
{
    float4 fragColor : SV_Target0;
};

[shader("fragment")]
PSOutput Main(PSInput input)
{
    PSOutput output;

    float3 color = Params.SceneColor.Sample(Params.SceneColorSampler, float2(1.0f - input.uv.x, input.uv.y)).rgb;

    if (Params.bEnableToneMapping != 0)
        color = float3(1.0f) - exp(-color);

    output.fragColor = float4(pow(color, float3(1.0f / Params.GammaCorrection)), 1.0f);
    return output;
}
