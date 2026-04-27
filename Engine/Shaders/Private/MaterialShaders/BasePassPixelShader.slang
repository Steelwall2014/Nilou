module BasePassPixelShader;
import RenderCore;
import BasePassCommon;

#ifndef ENABLE_IBL
#define ENABLE_IBL 0
#endif

#if ENABLE_IBL
struct FBasePassPixelShaderParameters
{
    SamplerCube IrradianceTexture;
    SamplerCube PrefilteredTexture;
    Sampler2D IBL_BRDF_LUT;
}
ParameterBlock<FBasePassPixelShaderParameters> Params;
#endif

struct BasePassPixelShaderOutput
{
    float3 SceneColor;
    float4 BaseColor;
    float3 RelativeWorldSpacePosition;
    float3 WorldSpaceNormal;
    float2 MetallicRoughness;
    float3 Emissive;
    EShadingModel ShadingModel;
};

float getMipLevelFromRoughness(float roughness)
{
    return roughness;// * (PrefilterEnvTextureNumMips-1);
}
float3 fresnelSchlickRoughness(float cosTheta, float3 F0, float roughness)
{
    return F0 + (max(float3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

float3 CalcIndirectLighting(IVertexShaderOutput vs_out, float3 baseColor, float metallic, float roughness)
{
    float3 V = normalize(-vs_out.GetRelativeWorldPosition());
    float3 N = normalize(vs_out.GetWorldNormal());
    if (dot(N, V) < 0)
        N = -N;
    float NdotV = max(dot(N, V), 0);
    float3 R = reflect(-V, N);
    float lod = getMipLevelFromRoughness(roughness);

#if ENABLE_IBL
    float3 irradiance = mytextureCube(Params.IrradianceTexture, R).rgb;
    float3 prefilteredColor = mytextureCubeLod(Params.PrefilteredTexture, R, lod).rgb;
    float2 envBRDF = Params.IBL_BRDF_LUT.Sample(float2(NdotV, roughness)).rg;
#endif

    float3 F0 = float3(0.04);
    F0 = lerp(F0, baseColor, metallic);
    float3 F = fresnelSchlickRoughness(NdotV, F0, roughness);
    float3 kS = F;
    float3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;

#if ENABLE_IBL
    float3 diffuse = irradiance * baseColor;
    float3 specular = prefilteredColor * (F * envBRDF.x + envBRDF.y);
    return kD * diffuse + specular;
#else
    return baseColor;
#endif
}

[shader("pixel")]
BasePassPixelShaderOutput MainPS<TMaterial : IMaterial>(BasePassVertexShaderOutput vs_out)
{
    TMaterial material = TMaterial(vs_out);
    BasePassPixelShaderOutput output;
    output.WorldSpaceNormal = material.GetWorldSpaceNormal();
    float4 GammaBaseColor = material.GetBaseColor();
    output.BaseColor = float4(GammaToLinear(GammaBaseColor.rgb), GammaBaseColor.a);
    output.RelativeWorldSpacePosition = vs_out.GetRelativeWorldPosition();
    output.MetallicRoughness.x = material.GetMetallic();
    output.MetallicRoughness.y = material.GetRoughness();
    output.Emissive = GammaToLinear(material.GetEmissive());
    output.ShadingModel = material.GetShadingModel();
    output.SceneColor = CalcIndirectLighting(vs_out, output.BaseColor.rgb, output.MetallicRoughness.x, output.MetallicRoughness.y);
    return output;
}
