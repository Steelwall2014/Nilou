#version 330 core
layout(location = 0) in vec3 POSITION;
layout(location = 1) in vec3 NORMAL;
layout(location = 2) in vec4 TANGENT;
layout(location = 3) in vec2 TEXCOORD_0;

out vec3 frag_position;
out vec3 frag_normal;
out vec4 frag_lightspace_pos[5];
out vec2 uv;
out mat3 TBN;

uniform mat4 model;
uniform mat4 VP;

#include "include/Light.glsl"

void main()
{
    gl_Position = VP * model * vec4(POSITION, 1.0f);
    frag_position = vec3(model * vec4(POSITION, 1.0f));
    vec3 normal = frag_normal = normalize(mat3(transpose(inverse(model))) * NORMAL);
    vec4 tangent = normalize(model * TANGENT);
    vec3 bitanget = normalize(cross(normal, tangent.xyz));
    //vec3 bitanget = cross(normal, tangent.xyz) * tangent.w;
    TBN = mat3(vec3(tangent), bitanget, normal);
    for (int i = 0; i < lightCount; i++)
    {
        if (lights[i].lightCastShadow)
            frag_lightspace_pos[i] = lights[i].lightVP * vec4(frag_position, 1.f);
    }
    uv = TEXCOORD_0;
}