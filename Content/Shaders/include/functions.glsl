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
    return texture(Cube, vec3(sampleVec.x, sampleVec.z, sampleVec.y));
}
#endif