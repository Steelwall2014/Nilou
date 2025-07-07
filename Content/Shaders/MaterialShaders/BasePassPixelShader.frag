#version 460 core
#include "../include/Macros.glsl"
layout (location = 0) out vec4 BaseColor;
layout (location = 1) out vec3 RelativeWorldSpacePosition;
layout (location = 2) out vec3 WorldSpaceNormal;
layout (location = 3) out vec2 MetallicRoughness;
layout (location = 4) out vec3 Emissive;
layout (location = 5) out uint ShadingModel;
/*
layout(set=SET_INDEX, binding=BINDING_INDEX, std140) uniform PIXEL_UNIFORM_BLOCK {
    uint MaterialShadingModel;
    uint PrefilterEnvTextureNumMips;
    float ReflectionProbeFactor;
};*/
//#include "../include/Maths.glsl"
//#include "../include/Light.glsl"
#include "../include/PBRFunctions.glsl"

// To be filled
//#include "../Materials/ColoredMaterial_Mat.glsl"

#if ENABLE_REFLECTION_PROBE
layout (set=SET_INDEX, binding=BINDING_INDEX) uniform samplerCube IrradianceTexture;

layout (set=SET_INDEX, binding=BINDING_INDEX) uniform samplerCube PrefilteredTexture;

layout (set=SET_INDEX, binding=BINDING_INDEX) uniform sampler2D IBL_BRDF_LUT;
#endif

//layout (std140) uniform FPrefilteredTextureBlock {
//    uint NumMips;
//};

layout (location = 0) in VS_Out vs_out;

#include "../include/functions.glsl"

float getMipLevelFromRoughness(float roughness)
{
    return roughness;// * (PrefilterEnvTextureNumMips-1);
}
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
} 

#if ENABLE_REFLECTION_PROBE
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
#endif

void main()
{
    WorldSpaceNormal = MaterialGetWorldSpaceNormal(vs_out);
//    vec3 TangentSpaceNormal = MaterialGetTangentSpaceNormal(vs_out).rgb;
//    TangentSpaceNormal = normalize(TangentSpaceNormal * 2.0f - 1.0f);   
//    WorldSpaceNormal = normalize(vs_out.TBN * TangentSpaceNormal);
    vec4 GammaBaseColor = MaterialGetBaseColor(vs_out);
    BaseColor = vec4(GammaToLinear(GammaBaseColor.rgb), GammaBaseColor.a);
    RelativeWorldSpacePosition = vs_out.RelativeWorldPosition;
    MetallicRoughness.x = MaterialGetMetallic(vs_out);
    MetallicRoughness.y = MaterialGetRoughness(vs_out);
    Emissive = GammaToLinear(MaterialGetEmissive(vs_out));
    ShadingModel = 0;//MaterialShadingModel;
#if ENABLE_REFLECTION_PROBE
    //Emissive += ReflectionProbeFactor*CalcIndirectLighting(BaseColor.rgb, MetallicRoughness.x, MetallicRoughness.y);
#endif
//    vec3 projCoords = frag_lightspace_pos[0].xyz / frag_lightspace_pos[0].w;
//    projCoords = projCoords * 0.5 + 0.5;
//    float closestDepth = texture(shadowMap, vec3(projCoords.xy, 0)).r; 
//    FragColor = vec4(closestDepth, closestDepth, closestDepth, 1);
}
