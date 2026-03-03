module atmosphere_skymap;

import unit_definitions;
import atmosphere_functions;
import atmosphere_definitions;
import atmosphere_constants;
import RenderCore;

struct FAtmosphereSkyMapParameters
{
    float3 SunLightDir;
    float2 SkyMapSize;
    Texture2D<float4> TransmittanceLUT;
    Texture3D<float4> SingleScatteringRayleighLUT;
    Texture3D<float4> SingleScatteringMieLUT;
    SamplerState linearSampler;
    RWTexture2D<float4> SkyMap;
};
ParameterBlock<FAtmosphereSkyMapParameters> Params;
ParameterBlock<AtmosphereParameters> ATMOSPHERE;

[shader("compute")]
[numthreads(32, 32, 1)]
void main(uint3 threadID : SV_DispatchThreadID)
{
    float3 sun_direction = -normalize(Params.SunLightDir);
    float2 xy = (float2(threadID.xy) + float2(0.5)) / Params.SkyMapSize;
    float3 v = GetViewVectorFromUV(xy);
    float3 earth_center = float3(0, 0, -ATMOSPHERE.bottom_radius);
    float3 color = GetSkyColor(ATMOSPHERE, Params.TransmittanceLUT, Params.SingleScatteringRayleighLUT, Params.SingleScatteringMieLUT, Params.linearSampler, -earth_center.z, v, 0.0 * m, sun_direction, ATMOSPHERE.sun_angular_radius);
    // color = hdr(color, 10);
    Params.SkyMap[threadID.xy] = float4(color, 1.0);
}

