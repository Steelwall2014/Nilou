#include "../include/BasePassCommon.glsl"

uniform sampler2D baseColorMap;
uniform sampler2D emissiveMap;
uniform sampler2D normalMap;
uniform sampler2D occlusionMap;
uniform sampler2D roughnessMetallicMap;

vec4 MaterialGetBaseColor(VS_Out vs_out)
{
    %s
}

vec3 MaterialGetEmissive(VS_Out vs_out)
{
    %s
}

vec3 MaterialGetWorldSpaceNormal(VS_Out vs_out)
{
    %s
}

vec4 MaterialGetOcclusion(VS_Out vs_out)
{
    %s
}

float MaterialGetRoughness(VS_Out vs_out)
{
    %s
}

float MaterialGetMetallic(VS_Out vs_out)
{
    %s
}

vec3 MaterialGetWorldSpaceOffset(VS_Out vs_out)
{
    return vec3(0);
}