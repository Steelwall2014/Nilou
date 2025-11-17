#version 460
#include "../include/Macros.glsl"
layout (location = 0) out vec4 FragColor;

layout (location = 0) in vec2 uv;

layout (binding=0) uniform sampler2D SceneColor;

#include "../include/PBRFunctions.glsl"

layout(binding=1, std140) uniform PIXEL_UNIFORM_BLOCK {
	float GammaCorrection;
	int bEnableToneMapping;
};

void main()
{
	vec3 color = texture(SceneColor, vec2(1-uv.x, uv.y)).rgb;
    if (bEnableToneMapping != 0)
		color = vec3(1.0) - exp(-color);
	FragColor = vec4(pow(color, vec3(1.0 / GammaCorrection)), 1);
}