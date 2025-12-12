float saturate(float x)
{
    return clamp(x, 0.f, 1.f);
}
float3 saturate(float3 x)
{
    return clamp(x, 0.f, 1.f);
}
float3 lerp(float3 from, float3 to, float t)
{
    return from + (to-from) * t;
}
float lerp(float from, float to, float t)
{
    return from + (to-from) * t;
}
float4 mytextureCube(SamplerCube Cube, float3 sampleVec)
{
    float4 color = Cube.Sample(sampleVec);
    // Sometimes it just returns NaN....
    // Weird....
    if (isnan(color.r) || isnan(color.g) || isnan(color.b) || isnan(color.a))
        return float4(0);
    return color;
}
float4 mytextureCubeLod(SamplerCube Cube, float3 sampleVec, float Lod)
{
    float4 color = Cube.SampleLevel(sampleVec, Lod);
    if (isnan(color.r) || isnan(color.g) || isnan(color.b) || isnan(color.a))
        return float4(0);
    return color;
}
float4 ClipToNDC(float4 ClipPosition)
{
    ClipPosition /= ClipPosition.w;
    // NDC depth range [0, 1], while other axis [-1, 1]
    ClipPosition.xy = ClipPosition.xy * 0.5 + 0.5;
    return ClipPosition;
}