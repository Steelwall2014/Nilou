#version 460

layout (std140) uniform FShadowDepthPassParameters {
    mat4 WorldToLight;
};

vec3 VertexFactoryGetWorldPosition();

void main()
{
	vec3 WorldPosition = VertexFactoryGetWorldPosition();
    gl_Position = WorldToLight * vec4(WorldPosition, 1);
}