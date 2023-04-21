#version 460
#include "../include/BasePassCommon.glsl"

uniform sampler2D baseColorTexture;
uniform sampler2D metallicRoughnessTexture;
uniform sampler2D emissiveTexture;
// uniform sampler2D normalTexture;

layout (std140) uniform FGLTFMaterialBlock {
    vec4 baseColorFactor;
    vec3 emissiveFactor;
    float metallicFactor;
    float roughnessFactor;
};

vec4 MaterialGetBaseColor(VS_Out vs_out)
{
    return texture(baseColorTexture, vs_out.TexCoords) * baseColorFactor;
}
vec3 MaterialGetEmissive(VS_Out vs_out)
{
    return texture(emissiveTexture, vs_out.TexCoords).rgb * emissiveFactor;
}
vec3 MaterialGetWorldSpaceNormal(VS_Out vs_out)
{
    // vec3 tangent_normal = texture(normalTexture, vs_out.TexCoords).rgb;
    vec3 tangent_normal = vec3(0.5, 0.5, 1.0);
    tangent_normal = normalize(tangent_normal * 2.0f - 1.0f);
    return normalize(vs_out.TBN * tangent_normal);
}
float MaterialGetRoughness(VS_Out vs_out)
{
    return 1;
}
float MaterialGetMetallic(VS_Out vs_out)
{
    return 0;
}
vec3 MaterialGetWorldSpaceOffset(VS_Out vs_out)
{
    return vec3(0);
}
        