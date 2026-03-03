implementing LightingPassPixelShader;

// float3 ApplySkyAtmosphere(FLightShaderParameters light, ShadingParameters params)
// {
//     if (light.lightType != ELightType.Directional)
//         return float3(0.0f);

//     float3 v             = -params.V;
//     float3 sun_direction  = params.L;
//     float  CameraHeightInKM = float(ViewParameters.CameraPosition.z / km);
//     if (CameraHeightInKM < 0.0f)
//         CameraHeightInKM = 0.0f;
//     if (v.z < 0.0f)
//     {
//         if (dot(normalize(v.xy), normalize(sun_direction.xy)) > cos(ATMOSPHERE.sun_angular_radius))
//         {
//             float c = cos(ATMOSPHERE.sun_angular_radius * 2.0f);
//             float s = sin(ATMOSPHERE.sun_angular_radius * 2.0f);
//             float2x2 rotation = float2x2(c, -s, s, c);
//             v = normalize(float3(mul(rotation, v.xy), 0.0f));
//         }
//         else
//         {
//             v = normalize(float3(v.xy, 0.0f));
//         }
//         CameraHeightInKM = 0.0f;
//     }
//     float shadow_length = 0.0f;
//     float r             = CameraHeightInKM - earth_center.z;
//     float3 color        = GetSkyColor(
//         ATMOSPHERE, TransmittanceLUT, ScatteringRayleighLUT, ScatteringMieLUT,
//         r, v, shadow_length, sun_direction, ATMOSPHERE.sun_angular_radius);
//     return color * light.lightIntensity;
// }
