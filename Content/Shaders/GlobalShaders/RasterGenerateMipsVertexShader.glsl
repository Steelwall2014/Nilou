#version 460 core

layout(location = 0) in vec4 InPosition;
layout(location = 1) in vec2 InUV;

layout(location = 0) out vec2 UV;

void main()
{
	vec4 OutPosition = InPosition;
	OutPosition.xy = -1.0f + 2.0f * InPosition.xy;
	gl_Position = OutPosition;
	UV = InUV;
}