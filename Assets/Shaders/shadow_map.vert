#version 330 core
layout(location = 0) in vec3 POSITION;
layout(location = 1) in vec3 NORMAL;

uniform mat4 lightVP;
uniform mat4 model;

void main()
{
    gl_Position = lightVP * model * vec4(POSITION, 1.0f);
}