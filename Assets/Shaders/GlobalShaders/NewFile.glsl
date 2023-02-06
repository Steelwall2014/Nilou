#version 460
#define USING_OPENGL 1
uniform sampler2D BaseColor;

uniform sampler2D Emissive;

layout (std140) uniform  FLightUniformBlock {
    FLightShaderParameters light;
};
uniform sampler2D MetallicRoughness;

uniform sampler2D WorldSpaceNormal;

uniform sampler2D WorldSpacePosition;

//#define STATIC_LIGHT_NUM 10
layout (location = 0) out vec4 FragColor;

in vec2 uv;

#ifndef MATHS_H
#define MATHS_H

#define PI (3.141592653589)

#endif

//layout (std140) buffer FLightSSBO {
//    FLightShaderParameters lights[STATIC_LIGHT_NUM];
//};

//layout (std140) uniform FLightUniformBlock {
//    FLightShaderParameters lights[STATIC_LIGHT_NUM];
//};
layout (std140) uniform FViewShaderParameters {
    mat4 WorldToView;
    mat4 ViewToClip;
    mat4 WorldToClip;
    mat4 ClipToWorld;
    vec3 CameraPosition;
    vec3 CameraDirection;
    ivec2 CameraResolution;
    float CameraNearClipDist;
    float CameraFarClipDist;
};

struct ShadingParams
{
    vec3 baseColor;
    vec3 emissive;
    vec3 worldSpacePosition;
    vec3 worldSpaceNormal;
    vec3 worldSpaceViewVector;
    float metallic;
    float roughness;
};

vec3 ApplyLight(FLightShaderParameters light, ShadingParams params)
{
    vec3 L;
    if (light.lightType == 2)   // Directional
        L = normalize(-light.lightDirection);
    else
        L = normalize(light.lightPosition - params.worldSpacePosition);
    vec3 N = params.worldSpaceNormal;
    vec3 V = params.worldSpaceViewVector;
    vec3 H = normalize(V + L);

    float NdotL = max(dot(N, L), 0.0);
    float NdotV = max(dot(N, V), 0.0);
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, params.baseColor.rgb, params.metallic);
    vec3 F = fresnel_Schlick(F0, clamp(dot(H, L), 0.0, 1.0));
    float G = GeometrySmith(NdotL, NdotV, params.roughness);
    float NDF = NDF_GGXTR(N, H, params.roughness);
    vec3 nominator = NDF * G * F;
    float denominator = 4.0 * max(NdotV, 0.0) * max(NdotL, 0.0) + 0.001;
    vec3 BRDF = nominator / denominator;

    float bias = max(0.01 * (1.0 - NdotL), 0.001);
    float visibility = 1;//ShadowCalculation(fragPosLightSpace, light.lightShadowMapLayerIndex, bias);

    float atten = 1;
    if (light.lightType == 3 || light.lightType == 1)   // Point or Spot
    {
        float dist = length(light.lightPosition - params.worldSpacePosition);
        atten *= apply_atten_curve(dist, light.lightDistAttenParams);
    }
    if (light.lightType == 1)       // Spot
    {
        float angle = acos(dot(normalize(light.lightDirection), -L));
        atten *= apply_atten_curve(angle, light.lightAngleAttenParams);
    }
    vec3 radiance = atten * vec3(light.lightColor) * light.lightIntensity;

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;

    kD *= 1.0 - params.metallic;

    return (kD * params.baseColor / PI + BRDF) * radiance * NdotL * visibility;
}

void main()
{
    ShadingParams params;
    params.baseColor = texture(BaseColor, uv).rgb;
    params.emissive = texture(Emissive, uv).rgb;
    params.worldSpacePosition = texture(WorldSpacePosition, uv).rgb;
    params.worldSpaceNormal = normalize(texture(WorldSpaceNormal, uv).rgb);
    params.worldSpaceViewVector = normalize(CameraPosition - params.worldSpacePosition);
    params.metallic = texture(MetallicRoughness, uv).r;
    params.roughness = texture(MetallicRoughness, uv).g;

    vec3 color = vec3(0);
//    for (int LightIndex = 0; LightIndex < STATIC_LIGHT_NUM; LightIndex++)
//    {
//        color += ApplyLight(lights[LightIndex], params);
//    }
    color = ApplyLight(light, params);
    FragColor = vec4(color, 1);
}

The program '[30252] Nilou.exe' has exited with code 0 (0x0).
