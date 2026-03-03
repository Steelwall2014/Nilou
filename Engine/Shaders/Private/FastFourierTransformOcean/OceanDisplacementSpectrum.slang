module OceanDisplacementSpectrum;
import RenderCore;
import OceanFastFourierTransformParameters;

struct FOceanDisplacementSpectrumParameters
{
    RWTexture2D<float2> GaussianRandomRT;
    RWTexture2D<float2> HeightSpectrumRT;
    RWTexture2D<float2> DisplaceXSpectrumRT;
    RWTexture2D<float2> DisplaceYSpectrumRT;
}
ParameterBlock<FOceanDisplacementSpectrumParameters> Params;
static const float g = 9.80665f;

//色散关系

float dispersion(float2 k)
{
    return sqrt(g * length(k));
}
//Donelan-Banner方向扩展
float DonelanBannerDirectionalSpreading(float2 k)
{
    float betaS;
    float omegap = 0.855f * g / OceanFFTParameters.WindSpeed;
    float ratio = dispersion(k) / omegap;

    if (ratio < 0.95f)
    {
        betaS = 2.61f * pow(ratio, 1.3f);
    }
    if(ratio >= 0.95f && ratio < 1.6f)
    {
        betaS = 2.28f * pow(ratio, -1.3f);
    }
    if(ratio > 1.6f)
    {
        float epsilon = -0.4f + 0.8393f * exp(-0.567f * log(ratio * ratio));
        betaS = pow(10, epsilon);
    }
    float theta = atan(k.y / k.x) - atan(-OceanFFTParameters.WindDirection.y / -OceanFFTParameters.WindDirection.x);

    return betaS / max(1e-7f, 2.0f * tanh(betaS * PI) * pow(cosh(betaS * theta), 2));
}
//Phillips谱
float phillips(float2 k)
{
    float V = OceanFFTParameters.WindSpeed;
	float L = (V * V) / g;
	float l = L / 1000.0f;				

	float kdotw = dot(k, -OceanFFTParameters.WindDirection);
	float k2 = max(0.001f, dot(k, k));			

	float P_h = OceanFFTParameters.Amplitude * exp(-1.0f / (k2 * L * L)) / (k2 * k2 * k2) * (kdotw * kdotw);

	if (kdotw < 0.0f) {
		P_h *= 0.07f;
	}

	return P_h * exp(-k2 * l * l);
}
float2 complexMultiply(float2 c1, float2 c2)
{
    float a = c1.x * c2.x - c1.y * c2.y;
    float b = c1.x * c2.y + c1.y * c2.x;
    return float2(a, b);
}
void CreateHeightDisplaceSpectrum(uint3 id)
{
    float Lxy = OceanFFTParameters.DisplacementTextureSize;
    float2 k = float2(2.0f * PI * (OceanFFTParameters.N/2.f - int(id.x)) / Lxy, 2.0f * PI * (OceanFFTParameters.N/2.f - int(id.y)) / Lxy);

    float2 gaussian = Params.GaussianRandomRT[id.xy];

    float2 hTilde0 = gaussian * sqrt(abs(phillips(k)) / 2.0f);
    float2 hTilde0Conj = gaussian * sqrt(abs(phillips(-k)) / 2.0f);
    hTilde0Conj.y *= -1.0f;

    float omega = dispersion(k) * OceanFFTParameters.Time;
    float c = cos(omega);
    float s = sin(omega);
    
    float2 h1 = complexMultiply(hTilde0, float2(c, s));
    float2 h2 = complexMultiply(hTilde0Conj, float2(c, -s));
    float2 HTilde = h1 + h2;
    
	k.x = float(OceanFFTParameters.N / 2 - id.x);
	k.y = float(OceanFFTParameters.N / 2 - id.y);
    k /= max(0.001f, length(k));

    float2 KxHTilde = complexMultiply(float2(0, -k.x), HTilde);
    float2 kyHTilde = complexMultiply(float2(0, -k.y), HTilde);
    Params.DisplaceXSpectrumRT[id.xy] = KxHTilde;
    Params.DisplaceYSpectrumRT[id.xy] = kyHTilde;
    Params.HeightSpectrumRT[id.xy] = HTilde;
}

[shader("compute")]
[numthreads(32, 32, 1)]
void Main(uint3 threadID : SV_DispatchThreadID)
{
    CreateHeightDisplaceSpectrum(threadID);
}
