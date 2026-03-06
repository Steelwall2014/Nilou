module OceanMaterial_Mat;
import RenderCore;
import OceanFastFourierTransformParameters;

struct FOceanMaterialParameters
{
    Sampler2D PerlinNoise;
    Sampler2D DisplaceTexture;
    Sampler2D NormalTexture;
}
ParameterBlock<FOceanMaterialParameters> Params;

static const float BLEND_START = 10.0;    // m
static const float BLEND_END = 300.0;  // m

struct OceanMaterial : IMaterial
{
    IVertexShaderOutput vs_out;
    __init(IVertexShaderOutput vs_out)
    {
        this.vs_out = vs_out;
    }
    float4 GetBaseColor()
    {
        // baseColor is taken as SubSurfaceColour in ocean surface rendering  
        float3 SubSurfaceColour = float3(18, 65, 76) / 255.0;
        return float4(LinearToGamma(SubSurfaceColour), 1);
    }
    float3 GetEmissive()
    {
        // emissive is taken as SubSurfaceBase, SubSurfaceSun and SubSurfaceSunFallOff in ocean surface rendering  
        float SubSurfaceBase = 0.33;
        float SubSurfaceSun = 1.13;
        float SubSurfaceSunFallOff = 5.0;
        return LinearToGamma(float3(SubSurfaceBase, SubSurfaceSun, SubSurfaceSunFallOff));
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
        // metallic and roughness are taken as sigmaXsq and sigmaYsq in ocean surface rendering
        float dist = length(this.vs_out.GetRelativeWorldPosition().xy);
        float sigmaFactor = clamp((BLEND_END - dist) / (BLEND_END - BLEND_START), 0.0, 1.0);
        float sigmaXsq = 0.007;//lerp(0.015, 0.0015, sigmaFactor);        
        return sigmaXsq;
    }
    float GetRoughness()
    {
        // metallic and roughness are taken as sigmaXsq and sigmaYsq in ocean surface rendering     
        float dist = length(this.vs_out.GetRelativeWorldPosition().xy);
        float sigmaFactor = clamp((BLEND_END - dist) / (BLEND_END - BLEND_START), 0.0, 1.0);
        float sigmaYsq = 0.007;//lerp(0.015, 0.0015, sigmaFactor);  
        return sigmaYsq;
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

