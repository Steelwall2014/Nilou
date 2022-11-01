#version 330 core
layout (location = 0) in vec3 POSITION;

out vec3 TexCoords;

uniform mat4 VP;

void main()
{
    TexCoords = POSITION;
    vec4 pos = VP * vec4(POSITION, 1.0);
    gl_Position = pos.xyww;
}