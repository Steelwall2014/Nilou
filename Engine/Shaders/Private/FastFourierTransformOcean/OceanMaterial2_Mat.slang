module OceanMaterial2_Mat;
import RenderCore;
import OceanFastFourierTransformParameters;

struct FOceanMaterial2Parameters
{
    Sampler2D PerlinNoise;
    Sampler2D DisplaceTexture;
    Sampler2D NormalTexture;
}
ParameterBlock<FOceanMaterial2Parameters> Params;

static const float BLEND_START = 10.0;    // m
static const float BLEND_END = 300.0;  // m

class OceanMaterial2 : IMaterial
{
    IVertexShaderOutput vs_out;
    __init(IVertexShaderOutput vs_out)
    {
        this.vs_out = vs_out;
    }
    float4 GetBaseColor()
    {
        return float4(LinearToGamma(float3(18, 65, 76) / 255.0), 1);
    }
    float3 GetEmissive()
    {
        return float3(0);
    }
    float3 GetWorldSpaceNormal()
    {
        float dist = length(this.vs_out.GetRelativeWorldPosition().xy);
        float factor = clamp((BLEND_END - dist) / (BLEND_END - BLEND_START), 0.0, 1.0);
        float3 WorldPosition = this.vs_out.GetRelativeWorldPosition() + ViewParameters.CameraPosition;
        float2 UV = float2(WorldPosition.xy / OceanFFTParameters.DisplacementTextureSize);
        
        const float3 perlinFrequency = float3(1.12, 0.59, 0.23);
        const float3 perlinGradient = float3(0.014, 0.016, 0.022);
        float2 p0 = Params.PerlinNoise.Sample(UV * perlinFrequency.x - OceanFFTParameters.WindDirection * OceanFFTParameters.Time * 0.06).rg;
        float2 p1 = Params.PerlinNoise.Sample(UV * perlinFrequency.y - OceanFFTParameters.WindDirection * OceanFFTParameters.Time * 0.06).rg;
        float2 p2 = Params.PerlinNoise.Sample(UV * perlinFrequency.z - OceanFFTParameters.WindDirection * OceanFFTParameters.Time * 0.06).rg;
        
        float2 perl = (p0 * perlinGradient.x + p1 * perlinGradient.y + p2 * perlinGradient.z);
        float3 grad = Params.NormalTexture.Sample(UV).xyz;
        grad.xy = lerp(perl, grad.xy, factor);

        return normalize(grad.xyz);
    }
    float GetMetallic()
    {
        return 0.0;
    }
    float GetRoughness()
    {
        return 0.4;
    }
    float3 GetWorldSpaceOffset()
    {
        float dist = length(this.vs_out.GetRelativeWorldPosition().xy);
        float factor = clamp((BLEND_END - dist) / (BLEND_END - BLEND_START), 0.0, 1.0);
        float3 WorldPosition = this.vs_out.GetRelativeWorldPosition() + ViewParameters.CameraPosition;
        float2 UV = float2(WorldPosition.xy / OceanFFTParameters.DisplacementTextureSize);

        float3 displacement = Params.DisplaceTexture.Sample(UV).rgb;
        
        //	const float3 perlinFrequency = float3(1.12, 0.59, 0.23);
        //	const float3 perlinAmplitude = float3(0.35, 0.42, 0.57);
        //	float p0 = PerlinNoise.Sample(UV * perlinFrequency.x - OceanFFTParameters.WindDirection * OceanFFTParameters.Time * 0.06).a;
        //	float p1 = PerlinNoise.Sample(UV * perlinFrequency.y - OceanFFTParameters.WindDirection * OceanFFTParameters.Time * 0.06).a;
        //	float p2 = PerlinNoise.Sample(UV * perlinFrequency.z - OceanFFTParameters.WindDirection * OceanFFTParameters.Time * 0.06).a;
        //
        //	float perl = dot(float3(p0, p1, p2), perlinAmplitude);
        //	displacement = lerp(float3(0.0, 0, perl), displacement, factor);

        return displacement;
    }
    EShadingModel GetShadingModel()
    {
        return EShadingModel.OceanSubsurface;
    }
};

