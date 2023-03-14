#version 460

#ifndef SM_Unlit
#define SM_Unlit (0)
#endif
#ifndef SM_DefaultLit
#define SM_DefaultLit (1)
#endif
#ifndef SM_OceanSubsurface
#define SM_OceanSubsurface (2)
#endif
#ifndef SM_SkyAtmosphere
#define SM_SkyAtmosphere (3)
#endif
#ifndef FrustumCount
#define FrustumCount (1)
#endif

layout (location = 0) out vec4 FragColor;

in vec2 uv;

uniform sampler2D BaseColor;
uniform sampler2D RelativeWorldSpacePosition;
uniform sampler2D WorldSpaceNormal;
uniform sampler2D MetallicRoughness;
uniform sampler2D Emissive;
uniform usampler2D ShadingModel;

#include "../include/LightShaderParameters.glsl"
layout (std140) uniform FLightUniformBlock {
    FLightShaderParameters light;
};

#include "../ShadingModels/DefaultLit.glsl"
#include "../ShadingModels/OceanSurface.glsl"
#include "../ShadingModels/SkyAtmosphere.glsl"

#include "../include/ShadowMapShaderParameters.glsl"
uniform sampler2DArray ShadowMaps;
layout (std140) uniform FShadowMappingBlock {
    FShadowMappingParameters Frustums[FrustumCount];
};



float ShadowCalculation(FLightShaderParameters light, vec3 RelativePosition, float bias)
{
    vec4 ViewSpacePosition = RelWorldToView * vec4(RelativePosition, 1);
    float depth = abs(ViewSpacePosition.z);
    if (depth > Frustums[FrustumCount-1].FrustumFar)
        return 1;
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


void main()
{
    ShadingParams params;
    params.baseColor = texture(BaseColor, uv).rgb;
    params.emissive = texture(Emissive, uv).rgb;
    params.relativePosition = texture(RelativeWorldSpacePosition, uv).rgb;

    params.N = normalize(texture(WorldSpaceNormal, uv).rgb);

    params.V = normalize(-params.relativePosition);

    vec3 RelativeLightPosition = vec3(light.lightPosition - CameraPosition);
    if (light.lightType == LT_Directional)
        params.L = normalize(-light.lightDirection);
    else 
        params.L = normalize(RelativeLightPosition - params.relativePosition);

    params.H = normalize(params.L + params.V);

    params.metallic = texture(MetallicRoughness, uv).r;
    params.roughness = texture(MetallicRoughness, uv).g;
    float bias = max(0.01 * (1.0 - dot(params.N, params.L)), 0.001);
    float visibility = ShadowCalculation(light, params.relativePosition, bias);

    Length r = params.relativePosition.z + ATMOSPHERE.bottom_radius + float(CameraPosition.z);
	Number mu = params.L.z;
    vec3 transmittance_to_sky = GetTransmittanceToTopAtmosphereBoundary(
          ATMOSPHERE, TransmittanceLUT, r, mu);
    FLightShaderParameters light_transmittanced = light;
    light_transmittanced.lightIntensity *= transmittance_to_sky;

    vec3 transmittance_to_frag;
    vec3 in_scatter = GetSkyRadianceToPoint(ATMOSPHERE,
        TransmittanceLUT, ScatteringRayleighLUT, ScatteringMieLUT, 
        vec3(0, 0, CameraPosition.z/km) - earth_center, params.relativePosition/km - earth_center, 0, params.L, transmittance_to_frag);

    uint MaterialShadingModel = texture(ShadingModel, uv).r;

    switch (MaterialShadingModel)
    {
        case SM_Unlit:
            FragColor = vec4(params.baseColor, 1);
            break;
        case SM_DefaultLit:
            FragColor = vec4(ApplyDefaultLit(light_transmittanced, params, visibility)*transmittance_to_frag + in_scatter*2.5, 1);
            break;
        case SM_OceanSubsurface:
            FragColor = vec4(ApplyOceanSubsurface(light_transmittanced, params)*transmittance_to_frag + in_scatter*2.5, 1);
            break;
        case SM_SkyAtmosphere:
            FragColor = vec4(ApplySkyAtmosphere(light_transmittanced, params), 1);
            break;
    }
}