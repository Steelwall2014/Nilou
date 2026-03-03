module OceanGaussianSpectrum;

import RenderCore;
import OceanFastFourierTransformParameters;

static const float g = 9.80665f;

struct FOceanGaussianSpectrumTextures
{
    RWTexture2D<float2> GaussianRandomRT;
}
ParameterBlock<FOceanGaussianSpectrumTextures> Params;

//随机数生成
uint wangHash(uint seed)
{
    seed = (seed ^ 61) ^(seed >> 16);
    seed *= 9;
    seed = seed ^(seed >> 4);
    seed *= 0x27d4eb2d;
    seed = seed ^(seed >> 15);
    return seed;
}
//均匀分布随机数生成[0,1)
float rand(uint rngState)
{
    // Xorshift算法
    rngState ^= (rngState << 13);
    rngState ^= (rngState >> 17);
    rngState ^= (rngState << 5);
    return rngState / 4294967296.0f;;
}
float2 gaussian(uint2 id)
{
    uint rngState = wangHash(id.y * OceanFFTParameters.N + id.x);
    float x1 = rand(rngState);
    float x2 = rand(rngState);

    x1 = max(1e-6f, x1);
    x2 = max(1e-6f, x2);
    float g1 = sqrt(-2.0f * log(x1)) * cos(2.0f * PI * x2);
    float g2 = sqrt(-2.0f * log(x1)) * sin(2.0f * PI * x2);

    return float2(g1, g2);
}
void ComputeGaussianRandom(uint3 id)
{
    float2 g = gaussian(id.xy);
    Params.GaussianRandomRT[id.xy] = float2(g.x, g.y);
}

[shader("compute")]
[numthreads(32, 32, 1)]
void Main(uint3 threadID : SV_DispatchThreadID)
{
    ComputeGaussianRandom(threadID);
}
