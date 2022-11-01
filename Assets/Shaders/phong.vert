#version 330 core
layout(location = 0) in vec3 POSITION;
layout(location = 1) in vec3 NORMAL;

out vec3 frag_position;
out vec3 frag_normal;

uniform mat4 model;
uniform mat4 VP;

void main()
{
    gl_Position = VP * model * vec4(POSITION, 1.0f);
    frag_position = vec3(model * vec4(POSITION, 1.0f));
    frag_normal = mat3(transpose(inverse(model))) * NORMAL;
}