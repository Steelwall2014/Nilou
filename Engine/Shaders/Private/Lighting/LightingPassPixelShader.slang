module LightingPassPixelShader;

import RenderCore;

__include "Light.slang";
__include "ShadowMapping.slang";
__include "ShadingModel/DefaultLit.slang";
__include "ShadingModel/OceanSurface.slang";
__include "ShadingModel/SkyAtmosphere.slang";

struct ShadingParameters
{
    float3 baseColor;
    float3 emissive;
    float3 relativePosition; // Pixel position in world space, relative to camera
    float3 N;
    float3 V;
    float3 L;
    float3 H;
    float  metallic;
    float  roughness;
};

struct SceneTextures
{
    Sampler2D<float4>      BaseColor;
    Sampler2D<float4>      RelativeWorldSpacePosition;
    Sampler2D<float4>      WorldSpaceNormal;
    Sampler2D<float4>      MetallicRoughness;
    Sampler2D<float4>      Emissive;
    Sampler2D<uint4>       ShadingModel;
}
ParameterBlock<SceneTextures> sceneTextures;
ParameterBlock<ShadowMappingParameters> shadowMapping;
ParameterBlock<Light> light;

struct PSInput
{
    float2 uv : TEXCOORD0;
};

struct PSOutput
{
    float4 fragColor : SV_Target0;
};

[shader("fragment")]
PSOutput Main(PSInput input)
{
    PSOutput output;

    ShadingParameters params;
    params.baseColor = sceneTextures.BaseColor.Sample(input.uv).rgb;
    params.emissive = sceneTextures.Emissive.Sample(input.uv).rgb;
    params.relativePosition = sceneTextures.RelativeWorldSpacePosition.Sample(input.uv).rgb;
    params.N = normalize(sceneTextures.WorldSpaceNormal.Sample(input.uv).rgb);
    params.V = normalize(-params.relativePosition);

    if (light.type == ELightType.Directional)
    {
        params.L = normalize(-light.direction);
    }
    else
    {
        float3 RelativeLightPosition = float3(light.position - ViewParameters.CameraPosition);
        params.L = normalize(RelativeLightPosition - params.relativePosition);
    }

    params.H = normalize(params.L + params.V);
    params.metallic = sceneTextures.MetallicRoughness.Sample(input.uv).r;
    params.roughness = sceneTextures.MetallicRoughness.Sample(input.uv).g;

    float bias       = max(0.01f * (1.0f - dot(params.N, params.L)), 0.001f);
    float visibility = shadowMapping.ShadowCalculation(params.relativePosition, bias);

    // Length r  = params.relativePosition.z + ATMOSPHERE.bottom_radius + float(ViewParameters.CameraPosition.z);
    // Number mu = params.L.z;
    // float3 transmittance_to_sky = GetTransmittanceToTopAtmosphereBoundary(
    //     ATMOSPHERE, TransmittanceLUT, r, mu);
    // FLightShaderParameters light_transmittanced = Light;
    // light_transmittanced.lightIntensity; // *= transmittance_to_sky;

    // float3 transmittance_to_frag;
    // float3 in_scatter = GetSkyRadianceToPoint(ATMOSPHERE,
    //     TransmittanceLUT, ScatteringRayleighLUT, ScatteringMieLUT,
    //     float3(0.0f, 0.0f, ViewParameters.CameraPosition.z / km) - earth_center,
    //     params.relativePosition / km - earth_center, 0.0f, params.L, transmittance_to_frag);

    EShadingModel MaterialShadingModel = (EShadingModel)sceneTextures.ShadingModel.Sample(input.uv).r;

    switch (MaterialShadingModel)
    {
        case EShadingModel.Unlit:
            output.fragColor = float4(params.baseColor, 1.0f);
            break;
        case EShadingModel.DefaultLit:
            output.fragColor = float4(ApplyDefaultLit(light, params, visibility) /**transmittance_to_frag + in_scatter*2.5*/, 1.0f);
            break;
        // TODO: 大气渲染要移到base pass里
        // case EShadingModel.OceanSubsurface:
        //     output.fragColor = float4(ApplyOceanSubsurface(light_transmittanced, params) /**transmittance_to_frag + in_scatter*2.5*/, 1.0f);
        //     break;
        // case EShadingModel.SkyAtmosphere:
        //     output.fragColor = float4(ApplySkyAtmosphere(light_transmittanced, params), 1.0f);
        //     break;
    }

    return output;
}
