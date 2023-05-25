#version 460
layout (location = 0) out vec4 FragColor;

layout (location = 0) in vec2 uv;

uniform sampler2D SceneColor;

#include "../include/PBRFunctions.glsl"

uniform float GammaCorrection;
uniform int bEnableToneMapping;

void main()
{
#if USING_OPENGL
	vec3 color = texture(SceneColor, vec2(1-uv.x, uv.y)).rgb;
#else
	vec3 color = texture(SceneColor, uv).rgb;
#endif
    if (bEnableToneMapping != 0)
		color = vec3(1.0) - exp(-color);
	FragColor = vec4(pow(color, vec3(1.0 / GammaCorrection)), 1);
}