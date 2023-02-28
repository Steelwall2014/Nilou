#version 460
layout (location = 0) out vec4 FragColor;

in vec2 uv;

uniform sampler2D BaseColor;
uniform sampler2D RelativeWorldSpacePosition;
uniform sampler2D WorldSpaceNormal;
uniform sampler2D MetallicRoughness;
uniform sampler2D Emissive;

#include "../include/LightShaderParameters.glsl"
#include "../include/PBRFunctions.glsl"
#include "../include/functions.glsl"

layout (std140) uniform FLightUniformBlock {
    FLightShaderParameters light;
};

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

#include "../include/ShadowMapShaderParameters.glsl"
uniform sampler2DArray ShadowMaps;
layout (std140) uniform FShadowMappingBlock {
    FShadowMappingParameters Frustums[FrustumCount];
};

float ShadowCalculation(FLightShaderParameters light, vec3 RelativePosition, float bias)
{
    vec4 ViewSpacePosition = RelWorldToView * vec4(RelativePosition, 1);
    float depth = abs(ViewSpacePosition.z);
    int FrustumIndex = -1;
    for (int i = FrustumCount-1; i >= 0; i--)
    {
        if (depth < Frustums[i].FrustumFar)
            FrustumIndex = i;
    }
    if (FrustumIndex != -1)
    {
        vec4 LightClipNDC = vec4(Frustums[FrustumIndex].WorldToClip * dvec4(RelativePosition+CameraPosition, 1));
        LightClipNDC /= LightClipNDC.w;
        LightClipNDC = LightClipNDC * 0.5 + 0.5;
        vec2 ShadowMapUV = LightClipNDC.xy;
        float SceneLightSpaceDepth = LightClipNDC.z;
        vec2 Resolution = vec2(Frustums[FrustumIndex].Resolution);
        float result = 0;
        int gridSize = 3;
        for (int i = -gridSize; i <= gridSize; i++)
        {
            for (int j = -gridSize; j <= gridSize; j++)
            {
                vec2 uv_offset = vec2(i, j) / Resolution;
                vec2 sampleUV = ShadowMapUV + uv_offset;

                vec2 texelPos = sampleUV * Resolution;
                vec2 texelPosFraction = fract(texelPos);

                vec2 samplePosCenter = floor(texelPos);
                samplePosCenter /= Resolution;

                vec4 ShadowMapDepth = textureGather(ShadowMaps, vec3(samplePosCenter, FrustumIndex));
                vec4 depth = step(vec4(SceneLightSpaceDepth), ShadowMapDepth + vec4(bias));
                float d1 = lerp(depth.w, depth.z, texelPosFraction.x);
                float d2 = lerp(depth.x, depth.y, texelPosFraction.x);
                float d = lerp(d1, d2, texelPosFraction.y);
                result += d;
            }
        }
        return clamp(result / float((gridSize * 2 + 1) * (gridSize * 2 + 1)), 0, 1);
    }

    return 0;
}

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
    F0 = mix(F0, params.baseColor.rgb, params.metallic);
    vec3 F = fresnel_Schlick(F0, clamp(dot(H, L), 0.0, 1.0));
    float G = GeometrySmith(NdotL, NdotV, params.roughness);
    float NDF = NDF_GGXTR(N, H, params.roughness);
    vec3 nominator = NDF * G * F;
    float denominator = 4.0 * max(NdotV, 0.0) * max(NdotL, 0.0) + 0.001; 
    vec3 BRDF = nominator / denominator;

    float bias = 0.002;//max(0.04 * (1.0 - NdotL), 0.004);
    float visibility = ShadowCalculation(light, params.relativePosition, bias);

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
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));  
    FragColor = vec4(color, 1);
}