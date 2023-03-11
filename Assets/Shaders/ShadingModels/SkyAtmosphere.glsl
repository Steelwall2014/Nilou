//#version 460
#ifndef SKY_ATMOSPHERE_H
#define SKY_ATMOSPHERE_H
#include "../include/LightShaderParameters.glsl"
#include "../SkyAtmosphere/atmosphere_functions.glsl"
#include "../include/ViewShaderParameters.glsl"
#include "../include/PBRFunctions.glsl"
#include "ShadingParams.glsl"

#include "SkyAtmosphereLUTs.glsl"

vec3 ApplySkyAtmosphere(FLightShaderParameters light, ShadingParams params)
{
    if (light.lightType != LT_Directional)
        return vec3(0);
    vec3 v = -params.V;
    vec3 sun_direction = params.L;
    float CameraHeightInKM = float(CameraPosition.z / km);
    if (CameraHeightInKM < 0) CameraHeightInKM = 0;
    if (v.z < 0.f)
    {
        if (dot(normalize(v.xy), normalize(sun_direction.xy)) > cos(ATMOSPHERE.sun_angular_radius))
        {
            float c = cos(ATMOSPHERE.sun_angular_radius*2);
            float s = sin(ATMOSPHERE.sun_angular_radius*2);
            mat2 rotation = mat2(
                c, -s, 
                s, c
            );
            v = normalize(vec3(rotation * v.xy, 0.f));
        }
        else
        {
            v = normalize(vec3(v.xy, 0.f));
        }
        CameraHeightInKM = 0;
    }
    float shadow_length = 0;
    float r = CameraHeightInKM - earth_center.z;
    vec3 color = GetSkyColor(
        ATMOSPHERE, TransmittanceLUT, ScatteringRayleighLUT, ScatteringMieLUT, 
        r, v, shadow_length, sun_direction, ATMOSPHERE.sun_angular_radius);
	color = HDR(color, 10);
    return color;
}
#endif