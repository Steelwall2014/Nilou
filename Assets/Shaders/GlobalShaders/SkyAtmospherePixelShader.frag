#version 460
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 SonarHUDColor;

in vec3 TexCoords;

#include "../SkyAtmosphere/unit_definitions.glsl"
#include "../SkyAtmosphere/atmosphere_definitions.glsl"

uniform vec3 cameraPos;
uniform vec3 SunLightDir;
uniform sampler2D TransmittanceLUT;
uniform sampler3D SingleScatteringRayleighLUT;
uniform sampler3D SingleScatteringMieLUT;

void main()
{        
    SonarHUDColor = vec4(0, 0, 0, 1);
    if (cameraPos.z < 0)
    {
        FragColor = vec4(0, 0, 0, 1.f);
        return;
    }
    vec3 sun_direction = -normalize(SunLightDir);
    vec3 v = normalize(TexCoords);
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
    float r = cameraPos.z / km - earth_center.z;
    vec3 color = GetSkyColor(
        ATMOSPHERE, TransmittanceLUT, SingleScatteringRayleighLUT, SingleScatteringMieLUT, 
        r, v, shadow_length, sun_direction, ATMOSPHERE.sun_angular_radius);
	color = hdr(color, 10);
    FragColor = vec4(color, 1.f);
}