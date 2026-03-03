module ViewElementPixelShader;

struct PSInput
{
    float3 color : COLOR;
};

struct PSOutput
{
    float4 fragColor : SV_Target0;
};

[shader("fragment")]
PSOutput Main(PSInput input)
{
    PSOutput output;
    output.fragColor = float4(input.color, 1.0f);
    return output;
}
