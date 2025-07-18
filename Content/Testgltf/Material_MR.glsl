#version 460
#include "../include/Macros.glsl"
#include "../include/BasePassCommon.glsl"
layout (std140, set=SET_INDEX, binding=BINDING_INDEX) uniform MAT_UNIFORM_BLOCK {
   vec4 baseColorFactor;
   vec3 emissiveFactor;
   float metallicFactor;
   float roughnessFactor;
};
layout (set=SET_INDEX, binding=BINDING_INDEX) uniform sampler2D baseColorTexture;
layout (set=SET_INDEX, binding=BINDING_INDEX) uniform sampler2D metallicRoughnessTexture;
layout (set=SET_INDEX, binding=BINDING_INDEX) uniform sampler2D normalTexture;
layout (set=SET_INDEX, binding=BINDING_INDEX) uniform sampler2D emissiveTexture;
vec3 MaterialGetWorldSpaceOffset(VS_Out vs_out)
{
    return vec3(0);
}
vec4 MaterialGetBaseColor(VS_Out vs_out)
{
   return texture(baseColorTexture, vs_out.TexCoords) * baseColorFactor;
}
float MaterialGetMetallic(VS_Out vs_out)
{
   return texture(metallicRoughnessTexture, vs_out.TexCoords).b * metallicFactor;
}
float MaterialGetRoughness(VS_Out vs_out)
{
   return texture(metallicRoughnessTexture, vs_out.TexCoords).g * roughnessFactor;
}
vec3 MaterialGetWorldSpaceNormal(VS_Out vs_out)
{
    vec3 tangent_normal = texture(normalTexture, vs_out.TexCoords).rgb;
    tangent_normal = normalize(tangent_normal * 2.0f - 1.0f);
    return normalize(vs_out.TBN * tangent_normal);
}
vec3 MaterialGetEmissive(VS_Out vs_out)
{
   return texture(emissiveTexture, vs_out.TexCoords).rgb * emissiveFactor;
}
