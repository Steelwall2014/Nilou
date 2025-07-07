#ifndef VERTEX_SHADER_OUTPUT
#define VERTEX_SHADER_OUTPUT
struct VS_Out 
{
    mat3 TBN;
    vec3 RelativeWorldPosition;
    vec4 ClipPosition;
    vec3 WorldNormal;
    vec3 WorldTangent;
    vec3 WorldBitangent;
    vec4 Color;
    vec2 TexCoords;
};
#endif