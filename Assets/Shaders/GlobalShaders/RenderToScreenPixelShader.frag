#version 460
layout (location = 0) out vec4 FragColor;

in vec2 uv;

uniform sampler2D SceneColor;

void main()
{
	FragColor = texture2D(SceneColor, uv);
}