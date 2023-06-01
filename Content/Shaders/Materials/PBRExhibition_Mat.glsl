#include "../include/BasePassCommon.glsl"

layout (std140, binding=0) uniform MAT_UNIFORM_BLOCK {
    float Red;
    float Green;
    float Blue;
    float Metallic;
    float Roughness;
};

vec4 MaterialGetBaseColor(VS_Out vs_out)
{
    return vec4(Red, Green, Blue, 1);
}
vec3 MaterialGetEmissive(VS_Out vs_out)
{
    return vec3(0);
}
vec3 MaterialGetWorldSpaceNormal(VS_Out vs_out)
{
    return normalize(vs_out.TBN * vec3(0, 0, 1));
}
float MaterialGetRoughness(VS_Out vs_out)
{
    return Roughness;
}
float MaterialGetMetallic(VS_Out vs_out)
{
    return Metallic;
}
vec3 MaterialGetWorldSpaceOffset(VS_Out vs_out)
{
    return vec3(0);
}
        