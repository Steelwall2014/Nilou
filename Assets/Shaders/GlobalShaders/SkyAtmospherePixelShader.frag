#version 460
layout (location = 0) out vec4 FragColor;

in vec2 uv;

#include "LightShaderParameters.glsl"
#include "../SkyAtmosphere/unit_definitions.glsl"
#include "../SkyAtmosphere/atmosphere_definitions.glsl"

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

layout (std140) uniform FLightUniformBlock {
    FLightShaderParameters light;
};

uniform sampler2D WorldSpacePosition;
uniform sampler2D TransmittanceLUT;
uniform sampler3D SingleScatteringRayleighLUT;
uniform sampler3D SingleScatteringMieLUT;

void main()
{        
    if (CameraPosition.z < 0)
    {
        FragColor = vec4(0, 0, 0, 1.f);
        return;
    }
    vec3 SunLightDir = light.lightDirection;
    vec3 sun_direction = -normalize(SunLightDir);
    vec4 FragWorldPosition = texture2D(WorldSpacePosition, uv);
    vec3 v = normalize(FragWorldPosition.xyz - CameraPosition);
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
    }
    float shadow_length = 0;
    float r = CameraPosition.z / km - earth_center.z;
    vec3 color = GetSkyColor(
        ATMOSPHERE, TransmittanceLUT, SingleScatteringRayleighLUT, SingleScatteringMieLUT, 
        r, v, shadow_length, sun_direction, ATMOSPHERE.sun_angular_radius);
	color = hdr(color, 10);
    FragColor = vec4(color, 1.f);
}