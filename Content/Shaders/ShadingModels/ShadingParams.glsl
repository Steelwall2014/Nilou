#ifndef SHADING_PARAMS_H
#define SHADING_PARAMS_H
struct ShadingParams
{
    vec3 baseColor;
    vec3 emissive;
    vec3 relativePosition;      // Pixel position in world space, relative to camera
    vec3 N;
    vec3 V;
    vec3 L;
    vec3 H;
    float metallic;
    float roughness;
};
#endif