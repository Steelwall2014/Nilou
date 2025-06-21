#version 460 core

layout(location = 0) in vec2 UV;

layout(location = 0) out vec4 OutColor;

layout (set=0, binding=0) uniform sampler2D MipInSRV;
layout (set=0, binding=1) uniform FParameters
{
	vec2 HalfTexelSize;
	float Level;
};

void main()
{
	OutColor = textureLod(MipInSRV, UV + HalfTexelSize, Level);
}