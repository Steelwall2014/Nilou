#ifndef BASIC_MATERIAL_H
#define BASIC_MATERIAL_H
#include "MaterialOutput.glsl"

#ifdef SUPPORTS_BASE_COLOR
uniform sampler2D baseColorMap;
#endif

#ifdef SUPPORTS_EMISSIVE
uniform sampler2D emissiveMap;
#endif

#ifdef SUPPORTS_NORMAL
uniform sampler2D normalMap;
#endif

#ifdef SUPPORTS_OCCLUSION
uniform sampler2D occlusionMap;
#endif

#ifdef SUPPORTS_ROUGHNESS_METAILLIC
uniform sampler2D roughnessMetallicMap;
#endif

MaterialOutput GetMaterialOutput(vec2 UV)
{
    MaterialOutput output;

#ifdef SUPPORTS_BASE_COLOR
    output.baseColor = texture(baseColorMap, UV);
#else
    output.baseColor = vec4(1, 1, 0, 1);
#endif

#ifdef SUPPORTS_EMISSIVE
    output.emissive = texture(emissiveMap, UV);
#else
    output.emissive = vec4(0);
#endif

#ifdef SUPPORTS_NORMAL
    output.normal = texture(normalMap, UV);
#else
    output.normal = vec3(0, 0, 1);
#endif

#ifdef SUPPORTS_OCCLUSION
    output.occlusion = texture(occlusionMap, UV);
#else
    output.occlusion = vec4(0);
#endif

#ifdef SUPPORTS_ROUGHNESS_METAILLIC
    output.roughness = texture(roughnessMetallicMap, UV).g;
    output.metallic = texture(roughnessMetallicMap, UV).b;
#else
    output.roughness = 1;
    output.metallic = 1;
#endif

    return output;
}
#endif