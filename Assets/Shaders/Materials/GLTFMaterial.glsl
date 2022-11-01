#include "../include/BasePassCommon.glsl"

uniform sampler2D baseColorMap;
uniform sampler2D emissiveMap;
uniform sampler2D normalMap;
uniform sampler2D occlusionMap;
uniform sampler2D metallicRoughnessMap;

vec4 MaterialGetBaseColor(VS_Out vs_out)
{
#if HAS_BASECOLOR
    return texture(baseColorMap, vs_out.TexCoords);
#else
    return vec4(0, 0, 0, 1);
#endif
}

vec3 MaterialGetEmissive(VS_Out vs_out)
{
#if HAS_EMISSIVE
    return texture(emissiveMap, vs_out.TexCoords).rgb;
#else
    return vec3(0, 0, 0);
#endif
}

vec3 MaterialGetWorldSpaceNormal(VS_Out vs_out)
{
#if HAS_NORMAL
    vec3 tangent_normal = texture(normalMap, vs_out.TexCoords).rgb;
    tangent_normal = normalize(tangent_normal * 2.0f - 1.0f);
    return normalize(vs_out.TBN * tangent_normal);
#else
    return normalize(vs_out.TBN * vec3(0, 0, 1));
#endif
}

vec4 MaterialGetOcclusion(VS_Out vs_out)
{
#if HAS_OCCLUSION
    return texture(occlusionMap, vs_out.TexCoords);
#else
    return vec4(0, 0, 0, 0);
#endif
}

float MaterialGetRoughness(VS_Out vs_out)
{
#if HAS_METALLICROUGHNESS
    return texture(metallicRoughnessMap, vs_out.TexCoords).g;
#else
    return 0.5;
#endif
}

float MaterialGetMetallic(VS_Out vs_out)
{
#if HAS_METALLICROUGHNESS
    return texture(roughnessMetallicMap, vs_out.TexCoords).b;
#else
    return 0.5;
#endif
}

vec3 MaterialGetWorldSpaceOffset(VS_Out vs_out)
{
    return vec3(0);
}