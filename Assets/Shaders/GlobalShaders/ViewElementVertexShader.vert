#version 460
layout(location = 0) in vec3 POSITION;
layout(location = 1) in vec3 COLOR;

out vec3 color;

#include "../include/ViewShaderParameters.glsl"

void main()
{
    gl_Position = RelWorldToClip * vec4(POSITION-CameraPosition, 1.0f);
    color = COLOR;
}