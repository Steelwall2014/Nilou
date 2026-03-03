#include "../include/BasePassCommon.glsl"
vec4 MaterialGetBaseColor(VS_Out vs_out)
{
    return vec4(0);
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
    return 0.5;
}
float MaterialGetMetallic(VS_Out vs_out)
{
    return 0.5;
}
vec3 MaterialGetWorldSpaceOffset(VS_Out vs_out)
{
    return vec3(0);
}
        