#version 460
#ifndef DEFAULT_LIT_H
#define DEFAULT_LIT_H

#include "../include/functions.glsl"
#include "../include/PBRFunctions.glsl"
#include "../include/LightShaderParameters.glsl"
#include "../include/ViewShaderParameters.glsl"
#include "ShadingParams.glsl"

vec3 ApplyDefaultLit(FLightShaderParameters light, ShadingParams params, float visibility)
{
    vec3 baseColor = params.baseColor.rgb;
    vec3 emissive = params.emissive.rgb;
    vec3 N = params.N;
    vec3 L = params.L;
    vec3 H = params.H;
    vec3 V = params.V;
    
    vec3 RelativeLightPosition = vec3(light.lightPosition - CameraPosition);
    float NdotL = max(dot(N, L), 0.0); 
    float NdotV = max(dot(N, V), 0.0); 
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, baseColor, params.metallic);
    vec3 F = fresnel_Schlick(F0, clamp(dot(H, L), 0.0, 1.0));
    float G = GeometrySmith(NdotL, NdotV, params.roughness);
    float NDF = NDF_GGXTR(N, H, params.roughness);
    vec3 nominator = NDF * G * F;
    float denominator = 4.0 * max(NdotV, 0.0) * max(NdotL, 0.0) + 0.001; 
    vec3 BRDF = nominator / denominator;


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
    vec3 radiance = atten * light.lightIntensity;

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;

    kD *= 1.0 - params.metallic;  

    vec3 color = (kD * baseColor / PI + BRDF) * radiance * NdotL * visibility + emissive;
    return color;
}
#endif