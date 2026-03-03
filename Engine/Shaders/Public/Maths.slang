implementing RenderCore;

public static const float PI = 3.1415926535;

public float saturate(float x)
{
    return clamp(x, 0.f, 1.f);
}

public float3 saturate(float3 x)
{
    return clamp(x, 0.f, 1.f);
}

public float3 lerp(float3 from, float3 to, float t)
{
    return from + (to-from) * t;
}

public float lerp(float from, float to, float t)
{
    return from + (to-from) * t;
}

public float4 mytextureCube(SamplerCube Cube, float3 sampleVec)
{
    float4 color = Cube.Sample(sampleVec);
    // Sometimes it just returns NaN....
    // Weird....
    if (isnan(color.r) || isnan(color.g) || isnan(color.b) || isnan(color.a))
        return float4(0);
    return color;
}

public float4 mytextureCubeLod(SamplerCube Cube, float3 sampleVec, float Lod)
{
    float4 color = Cube.SampleLevel(sampleVec, Lod);
    if (isnan(color.r) || isnan(color.g) || isnan(color.b) || isnan(color.a))
        return float4(0);
    return color;
}

public float4 ClipToNDC(float4 ClipPosition)
{
    ClipPosition /= ClipPosition.w;
    // NDC depth range [0, 1], while other axis [-1, 1]
    ClipPosition.xy = ClipPosition.xy * 0.5 + 0.5;
    return ClipPosition;
}

public float3 fresnel_Schlick(float3 R0, float cosTheta)
{
    return R0 + (1-R0) * pow(1-cosTheta, 5.0f);
}

public float fresnel_Schlick(float R0, float cosTheta)
{
    return R0 + (1-R0) * pow(1-cosTheta, 5.0f);
}

public float NDF_GGXTR(float3 N, float3 H, float roughness)
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

public float GeometrySchlickGGX(float NdotV, float roughness)
{
    // SIGGRAPH 2013 UE4
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

public float GeometrySmith(float NdotL, float NdotV, float roughness)
{
//    float NdotV = max(dot(N, V), 0.0);
//    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

public float3 GammaToLinear(float3 GammaColor)
{
    return pow(GammaColor, float3(2.2));
}

public float3 LinearToGamma(float3 LinearColor)
{
    LinearColor = LinearColor / (LinearColor + float3(1));
    return pow(LinearColor, float3(1/2.2));
}

public float3 HDR(float3 L, float hdrExposure) {
//    float hdrExposure = 12;
//    L = L * hdrExposure;
//    L.r = L.r < 1.413 ? pow(L.r * 0.38317, 1.0 / 2.2) : 1.0 - exp(-L.r);
//    L.g = L.g < 1.413 ? pow(L.g * 0.38317, 1.0 / 2.2) : 1.0 - exp(-L.g);
//    L.b = L.b < 1.413 ? pow(L.b * 0.38317, 1.0 / 2.2) : 1.0 - exp(-L.b);
//    return L;
    float3 mapped = float3(1.0) - exp(-L * hdrExposure);
    mapped = pow(mapped, float3(1.0 / 2.2));

    return mapped;
}
