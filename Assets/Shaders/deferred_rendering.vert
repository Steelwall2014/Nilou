#version 330 core
layout(location = 0) in vec3 POSITION;
layout(location = 1) in vec2 TEXCOORD_0;

uniform vec3 CameraRay[4];

out vec2 uv;
out vec3 cameraRay;

void main()
{
    gl_Position = vec4(POSITION, 1.0f);
    uv = TEXCOORD_0;
    cameraRay = CameraRay[gl_VertexID];
}