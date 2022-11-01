#include "Maths.glsl"
vec3 fresnel_Schlick(vec3 R0, float cosTheta)
{
    return R0 + (1-R0) * pow(1-cosTheta, 5.0f);
}
float fresnel_Schlick(float R0, float cosTheta)
{
    return R0 + (1-R0) * pow(1-cosTheta, 5.0f);
}
float NDF_GGXTR(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom    = a2;
    float denom  = (NdotH2 * (a2 - 1.0) + 1.0);
    denom        = PI * denom * denom;

    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    // SIGGRAPH 2013£ºUE4
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
float GeometrySmith(float NdotL, float NdotV, float roughness)
{
//    float NdotV = max(dot(N, V), 0.0);
//    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}