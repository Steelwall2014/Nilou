#version 460 core

layout(location = 0) in vec4 InPosition;
layout(location = 1) in vec2 InUV;

layout(location = 0) out vec2 UV;

void main()
{
	gl_Position = InPosition;
	UV = InUV;
}