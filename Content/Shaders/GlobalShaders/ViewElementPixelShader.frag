#version 460
layout (location = 0) out vec4 FragColor;

in vec3 color;

void main()
{
	FragColor = vec4(color, 1);
}