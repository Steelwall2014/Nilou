#ifndef FUNCTIONS_H
#define FUNCTIONS_H
float saturate(float x)
{
    return clamp(x, 0.f, 1.f);
}
vec3 saturate(vec3 x)
{
    return clamp(x, 0.f, 1.f);
}
vec3 lerp(vec3 from, vec3 to, float t)
{
    return from + (to-from) * t;
}
float lerp(float from, float to, float t)
{
    return from + (to-from) * t;
}
vec4 mytextureCube(samplerCube Cube, vec3 sampleVec)
{
#if RHI_API == RHI_VULKAN
    vec4 color = texture(Cube, sampleVec);
#elif RHI_API == RHI_OPENGL
    vec4 color = texture(Cube, sampleVec.xzy);
#endif
    // Sometimes it just returns NaN....
    // Weird....
    if (isnan(color.r) || isnan(color.g) || isnan(color.b) || isnan(color.a))
        return vec4(0);
    return color;
}
vec4 mytextureCubeLod(samplerCube Cube, vec3 sampleVec, float Lod)
{
#if RHI_API == RHI_VULKAN
    vec4 color = textureLod(Cube, sampleVec, Lod);
#elif RHI_API == RHI_OPENGL
    vec4 color = textureLod(Cube, sampleVec.xzy, Lod);
#endif
    if (isnan(color.r) || isnan(color.g) || isnan(color.b) || isnan(color.a))
        return vec4(0);
    return color;
}
vec4 ClipToNDC(vec4 ClipPosition)
{
    ClipPosition /= ClipPosition.w;
    // NDC depth range [0, 1], while other axis [-1, 1]
    ClipPosition.xy = ClipPosition.xy * 0.5 + 0.5;
    return ClipPosition;
}
#endif