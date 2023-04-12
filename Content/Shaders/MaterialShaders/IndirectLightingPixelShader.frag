#version 460 core
layout (location = 4) out vec3 Emissive;

uniform uint MaterialShadingModel;

uniform uint PrefilterEnvTextureNumMips;
// To be filled
//#include "../Materials/ColoredMaterial_Mat.glsl"

uniform sampler2D BaseColorTexture;

uniform samplerCube IrradianceTexture;

uniform samplerCube PrefilteredTexture;

uniform sampler2D IBL_BRDF_LUT;

in VS_Out vs_out;

#include "../include/functions.glsl"

float getMipLevelFromRoughness(float roughness)
{
    return roughness * (PrefilterEnvTextureNumMips-1);
}
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
} 

vec3 CalcIndirectLighting(vec3 baseColor, float metallic, float roughness)
{
    vec3 V = normalize(-vs_out.RelativeWorldPosition);
    vec3 N = normalize(vs_out.WorldNormal);
    if (dot(N, V) < 0)
        N = -N;
    float NdotV = max(dot(N, V), 0);
    vec3 R = reflect(-V, N);
    float lod = getMipLevelFromRoughness(roughness);

    vec3 irradiance = mytextureCube(IrradianceTexture, R).rgb;
    vec3 prefilteredColor = mytextureCubeLod(PrefilteredTexture, R, lod).rgb;
    vec2 envBRDF = texture(IBL_BRDF_LUT, vec2(NdotV, roughness)).rg;

    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, baseColor, metallic);
    vec3 F = fresnelSchlickRoughness(NdotV, F0, roughness);
    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;  

    vec3 diffuse = irradiance * baseColor;
    vec3 specular = prefilteredColor * (F * envBRDF.x + envBRDF.y);
    return kD * diffuse + specular;
}

void main()
{
//    Emissive = CalcIndirectLighting(BaseColor.rgb, MetallicRoughness.x, MetallicRoughness.y);
}
