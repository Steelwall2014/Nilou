#version 460
layout(location = 0) in vec3 POSITION;
layout(location = 1) in vec2 TEXCOORD_0;

layout(location = 0) out vec2 uv;

void main()
{
    gl_Position = vec4(POSITION, 1.0f);
    uv = TEXCOORD_0;
#if RHI_API == RHI_VULKAN
    uv.y = 1-uv.y;
#endif
}