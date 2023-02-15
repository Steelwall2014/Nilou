#version 460
layout (location = 0) out vec4 FragColor;

in vec2 uv;

#include "LightShaderParameters.glsl"
#include "../SkyAtmosphere/unit_definitions.glsl"
#include "../SkyAtmosphere/atmosphere_definitions.glsl"
#include "../include/ViewShaderParameters.glsl"

layout (std140) uniform FLightUniformBlock {
    FLightShaderParameters light;
};

uniform sampler2D TransmittanceLUT;
uniform sampler3D SingleScatteringRayleighLUT;
uniform sampler3D SingleScatteringMieLUT;

void main()
{        
//    if (CameraPosition.z < 0)
//    {
//        FragColor = vec4(0, 0, 0, 1.f);
//        return;
//    }
    vec3 SunLightDir = light.lightDirection;
    vec3 sun_direction = -normalize(SunLightDir);
    vec4 ndc = vec4(uv*2-1, gl_FragCoord.z*2-1, 1.0);
    vec4 worldPos = ClipToWorld * ndc;
    worldPos /= worldPos.w;
    vec3 v = normalize(worldPos.xyz);
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
        ATMOSPHERE, TransmittanceLUT, SingleScatteringRayleighLUT, SingleScatteringMieLUT, 
        r, v, shadow_length, sun_direction, ATMOSPHERE.sun_angular_radius);
	color = hdr(color, 10);
    FragColor = vec4(color, 1.f);
}