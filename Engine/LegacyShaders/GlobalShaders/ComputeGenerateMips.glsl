#version 460 core

layout(local_size_x = 16, local_size_y = 16) in;

layout(binding=0) uniform sampler2D MipInSRV;
layout(binding=1) writeonly uniform image2D MipOutUAV;

void main()
{
	ivec2 OutTextureSize = imageSize(MipOutUAV);
	uint px = gl_GlobalInvocationID.x;
	uint py = gl_GlobalInvocationID.y;
	if (px >= OutTextureSize.x || py >= OutTextureSize.y)
	{
		return;
	}

	vec2 uv = (vec2(px, py) + vec2(0.5)) / vec2(OutTextureSize);
	vec4 Out = texture(MipInSRV, uv);
	imageStore(MipOutUAV, ivec2(px, py), Out);
}