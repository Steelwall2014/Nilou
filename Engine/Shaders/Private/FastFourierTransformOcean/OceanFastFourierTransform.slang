module OceanFastFourierTransform;
import RenderCore;
import OceanFastFourierTransformParameters;
static const float g = 9.80665f;

struct FOceanFastFourierTransformTextures
{
    RWTexture2D<float2> InputRT;
    RWTexture2D<float2> OutputRT;
}
ParameterBlock<FOceanFastFourierTransformTextures> Params;

struct FOceanFFTButterflyBlock
{
    uint Ns;
};
uniform ParameterBlock<FOceanFFTButterflyBlock> OceanFFTButterflyBlock;

float2 complexMultiply(float2 c1, float2 c2)
{
    float a = c1.x * c2.x - c1.y * c2.y;
    float b = c1.x * c2.y + c1.y * c2.x;
    return float2(a, b);
}
float2 exp_theta(float theta) {
    return float2(cos(theta), sin(theta));
}
void FFTHorizontal(uint3 id)
{
    int2 l_in = int2(id.xy);
    int2 r_in = int2(id.xy);
    l_in.x = int(floor(id.x / (OceanFFTButterflyBlock.Ns*2)) * OceanFFTButterflyBlock.Ns + id.x % OceanFFTButterflyBlock.Ns);
    r_in.x = l_in.x + int(OceanFFTParameters.N) / 2;

    float theta = -2.0f * PI / (OceanFFTButterflyBlock.Ns * 2.0f) * id.x;
    float2 w = exp_theta(theta);

    float2 f0 = Params.InputRT[l_in];
    float2 f1 = Params.InputRT[r_in];

    float2 res = f0 + complexMultiply(w, f1);
    
    Params.OutputRT[id.xy] = res;
}
void FFTVertical(uint3 id)
{
    int2 l_in = int2(id.xy);
    int2 r_in = int2(id.xy);
    l_in.y = int(floor(id.y / (OceanFFTButterflyBlock.Ns*2)) * OceanFFTButterflyBlock.Ns + id.y % OceanFFTButterflyBlock.Ns);
    r_in.y = l_in.y + int(OceanFFTParameters.N) / 2;

    float theta = -2.0f * PI / (OceanFFTButterflyBlock.Ns * 2.0f) * id.y;
    float2 w = exp_theta(theta);

    float2 f0 = Params.InputRT[l_in];
    float2 f1 = Params.InputRT[r_in];

    float2 res = f0 + complexMultiply(w, f1);

    Params.OutputRT[id.xy] = res;
}

[shader("compute")]
[numthreads(32, 32, 1)]
void MainHorizontal(uint3 threadID : SV_DispatchThreadID)
{
    FFTHorizontal(threadID);
}

[shader("compute")]
[numthreads(32, 32, 1)]
void MainVertical(uint3 threadID : SV_DispatchThreadID)
{
    FFTVertical(threadID);
}
