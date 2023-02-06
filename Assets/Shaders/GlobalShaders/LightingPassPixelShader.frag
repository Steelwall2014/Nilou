#version 460
//#define STATIC_LIGHT_NUM 10
layout (location = 0) out vec4 FragColor;

in vec2 uv;

uniform sampler2D BaseColor;
uniform sampler2D RelativeWorldSpacePosition;
uniform sampler2D WorldSpaceNormal;
uniform sampler2D MetallicRoughness;
uniform sampler2D Emissive;

#include "LightShaderParameters.glsl"
#include "../include/PBRFunctions.glsl"

//layout (std140) buffer FLightSSBO {
//    FLightShaderParameters lights[STATIC_LIGHT_NUM];
//};

//layout (std140) uniform FLightUniformBlock {
//    FLightShaderParameters lights[STATIC_LIGHT_NUM];
//};
layout (std140) uniform FLightUniformBlock {
    FLightShaderParameters light;
};

// WorldToClip = ViewToClip * WorldToView
#include "../include/ViewShaderParameters.glsl"

struct ShadingParams
{
    vec3 baseColor;
    vec3 emissive;
    vec3 relativePosition;      // Pixel position in world space, relative to camera
    vec3 worldSpaceNormal;
    vec3 worldSpaceViewVector;
    float metallic;
    float roughness;
};

vec3 ApplyLight(FLightShaderParameters light, ShadingParams params)
{
    vec3 RelativeLightPosition = vec3(light.lightPosition - CameraPosition);
    vec3 L;
    if (light.lightType == LT_Directional)
        L = normalize(-light.lightDirection);
    else 
        L = normalize(RelativeLightPosition - params.relativePosition);
    vec3 N = params.worldSpaceNormal;
    vec3 V = params.worldSpaceViewVector;
    vec3 H = normalize(V + L);

    float NdotL = max(dot(N, L), 0.0); 
    float NdotV = max(dot(N, V), 0.0); 
    vec3 F0 = vec3(0.04); 
    F0 = F0 * 0.5 + params.baseColor.rgb * 0.5;//mix(F0, params.baseColor.rgb, params.metallic);
    vec3 F = fresnel_Schlick(F0, clamp(dot(H, L), 0.0, 1.0));
    float G = GeometrySmith(NdotL, NdotV, params.roughness);
    float NDF = NDF_GGXTR(N, H, params.roughness);
    vec3 nominator = NDF * G * F;
    float denominator = 4.0 * max(NdotV, 0.0) * max(NdotL, 0.0) + 0.001; 
    vec3 BRDF = nominator / denominator;

    float bias = max(0.01 * (1.0 - NdotL), 0.001);
    float visibility = 1;//ShadowCalculation(fragPosLightSpace, light.lightShadowMapLayerIndex, bias);

    float atten = 1;
    if (light.lightType == LT_Point || light.lightType == LT_Spot)   // Point or Spot
    {
        float dist = length(RelativeLightPosition);
        atten *= apply_atten_curve(dist, light.lightDistAttenParams);
    }
    if (light.lightType == LT_Spot)       // Spot 
    {
        float angle = acos(dot(normalize(light.lightDirection), -L));
        atten *= apply_atten_curve(angle, light.lightAngleAttenParams);
    } 
    vec3 radiance = atten * vec3(light.lightColor) * light.lightIntensity;

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;

    kD *= 1.0 - params.metallic;  

    return (kD * params.baseColor / PI + BRDF) * radiance * NdotL * visibility + params.emissive;
}

void main()
{
    ShadingParams params;
    params.baseColor = texture(BaseColor, uv).rgb;
    params.emissive = texture(Emissive, uv).rgb;
    params.relativePosition = texture(RelativeWorldSpacePosition, uv).rgb;
    params.worldSpaceNormal = normalize(texture(WorldSpaceNormal, uv).rgb);
    params.worldSpaceViewVector = normalize(-params.relativePosition);
    params.metallic = texture(MetallicRoughness, uv).r;
    params.roughness = texture(MetallicRoughness, uv).g;

    vec3 color = ApplyLight(light, params);
    FragColor = vec4(color, 1);
}