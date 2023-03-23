#version 460
layout (location = 0) out vec4 FragColor;

in vec2 uv;

uniform sampler2D SceneColor;

#include "../include/PBRFunctions.glsl"

void main()
{
#if USING_OPENGL
	vec3 color = texture(SceneColor, vec2(1-uv.x, uv.y)).rgb;
#else
	vec3 color = texture(SceneColor, uv).rgb;
#endif
	FragColor = vec4(HDR(color, 1), 1);
}