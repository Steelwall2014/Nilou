#version 460
#include "../include/Macros.glsl"
#define baseColorFactor_r (1.000000)
#define baseColorFactor_g (1.000000)
#define baseColorFactor_b (1.000000)
#define baseColorFactor_a (1.000000)
#define emissiveFactor_r (1.000000)
#define emissiveFactor_g (1.000000)
#define emissiveFactor_b (1.000000)
#define metallicFactor (1.000000)
#define roughnessFactor (1.000000)

#include "../include/BasePassCommon.glsl"

layout (set=MATERIAL_SET_INDEX, binding=0) uniform sampler2D baseColorTexture;
layout (set=MATERIAL_SET_INDEX, binding=1) uniform sampler2D metallicRoughnessTexture;
layout (set=MATERIAL_SET_INDEX, binding=2) uniform sampler2D emissiveTexture;
layout (set=MATERIAL_SET_INDEX, binding=3) uniform sampler2D normalTexture;

vec4 MaterialGetBaseColor(VS_Out vs_out)
{
    vec4 baseColorFactor = vec4(baseColorFactor_r, baseColorFactor_g, baseColorFactor_b, baseColorFactor_a);
    return texture(baseColorTexture, vs_out.TexCoords) * baseColorFactor;
}
vec3 MaterialGetEmissive(VS_Out vs_out)
{
    vec3 emissiveFactor = vec3(emissiveFactor_r, emissiveFactor_g, emissiveFactor_b);
    return texture(emissiveTexture, vs_out.TexCoords).rgb * emissiveFactor;
}
vec3 MaterialGetWorldSpaceNormal(VS_Out vs_out)
{
    vec3 tangent_normal = texture(normalTexture, vs_out.TexCoords).rgb;
    tangent_normal = normalize(tangent_normal * 2.0f - 1.0f);
    return normalize(vs_out.TBN * tangent_normal);
}
float MaterialGetRoughness(VS_Out vs_out)
{
    return texture(metallicRoughnessTexture, vs_out.TexCoords).g * roughnessFactor;
}
float MaterialGetMetallic(VS_Out vs_out)
{
    return texture(metallicRoughnessTexture, vs_out.TexCoords).b * metallicFactor;
}
vec3 MaterialGetWorldSpaceOffset(VS_Out vs_out)
{
    return vec3(0);
}
        