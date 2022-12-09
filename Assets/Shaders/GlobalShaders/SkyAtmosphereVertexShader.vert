#version 460
layout (location = 0) in vec3 POSITION;

out vec3 TexCoords;

layout (std140) uniform FViewShaderParameters {
    mat4 WorldToView;
    mat4 ViewToClip;
    mat4 WorldToClip;
    vec3 CameraPosition;
    vec3 CameraDirection;
};

void main()
{
    TexCoords = POSITION;
    vec4 pos = WorldToClip * vec4(POSITION, 1.0);
    gl_Position = pos.xyww;
}