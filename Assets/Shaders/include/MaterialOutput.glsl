#ifndef MATERIAL_OUTPUT_H
#define MATERIAL_OUTPUT_H

struct MaterialOutput
{
    vec4 baseColor;
    vec4 emissive;
    vec3 normal;
    vec4 occlusion;
    float roughness;
    float metallic;
};

#endif