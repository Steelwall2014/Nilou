module PrefilteredEnvTextureShader;

import RenderCore;

// Vulkan cube face forward vectors
static const float3 ForwardVectors[6] = {
    float3( 1,  0,  0),  // +X
    float3(-1,  0,  0),  // -X
    float3( 0,  1,  0),  // +Y
    float3( 0, -1,  0),  // -Y
    float3( 0,  0,  1),  // +Z
    float3( 0,  0, -1),  // -Z
};

static const float3 UpVectors[6] = {
    float3(0,  1,  0),
    float3(0,  1,  0),
    float3(0,  0, -1),
    float3(0,  0,  1),
    float3(0,  1,  0),
    float3(0,  1,  0),
};

float RadicalInverse_VdC(uint bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

float2 Hammersley(uint i, uint N)
{
    return float2(float(i) / float(N), RadicalInverse_VdC(i));
}

float3 ImportanceSampleGGX(float2 Xi, float3 N, float roughness)
{
    float a = roughness * roughness;

    float phi      = 2.0 * PI * Xi.x;
    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a * a - 1.0) * Xi.y));
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

    // Spherical coordinates to tangent-space Cartesian
    float3 H;
    H.x = cos(phi) * sinTheta;
    H.y = sin(phi) * sinTheta;
    H.z = cosTheta;

    // Tangent space to world space
    float3 up        = abs(N.z) < 0.999 ? float3(0.0, 0.0, 1.0) : float3(1.0, 0.0, 0.0);
    float3 tangent   = normalize(cross(up, N));
    float3 bitangent = cross(N, tangent);

    float3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
    return normalize(sampleVec);
}

struct FPrefilteredEnvTextureParameters
{
    SamplerCube              EnvironmentTexture;
    RWTexture2DArray<float4> PrefilteredTexture;
    int                      TextureSize;
    float                    roughness;
};
ParameterBlock<FPrefilteredEnvTextureParameters> Params;

[shader("compute")]
[numthreads(32, 32, 1)]
void Main(uint3 threadID : SV_DispatchThreadID)
{
    for (int face = 0; face < 6; face++)
    {
        float3 up      = UpVectors[face];
        float3 forward = ForwardVectors[face];
        float3 right   = cross(up, forward);

        float3 center     = forward;
        float3 lower_left = center - up - right;
        float3 N = lower_left
            + (threadID.x * 2.0 / float(Params.TextureSize)) * right
            + (threadID.y * 2.0 / float(Params.TextureSize)) * up;

        // Assume N = R = V (split-sum approximation)
        float3 R = N;
        float3 V = R;

        // Rebuild tangent basis with N as the normal
        right = cross(up, N);
        up    = cross(N, right);

        const uint SAMPLE_COUNT    = 1024u;
        float      totalWeight     = 0.0;
        float3     prefilteredColor = float3(0.0);

        for (uint j = 0u; j < SAMPLE_COUNT; ++j)
        {
            float2 Xi = Hammersley(j, SAMPLE_COUNT);
            float3 H  = ImportanceSampleGGX(Xi, N, Params.roughness);
            float3 L  = normalize(2.0 * dot(V, H) * H - V);

            float NdotL = max(dot(N, L), 0.0);
            if (NdotL > 0.0)
            {
                prefilteredColor += mytextureCubeLod(Params.EnvironmentTexture, L, 0.0).rgb * NdotL;
                totalWeight      += NdotL;
            }
        }
        prefilteredColor = prefilteredColor / totalWeight;

        // Vulkan: flip Y axis
        Params.PrefilteredTexture[int3(threadID.x, Params.TextureSize - 1 - threadID.y, face)] =
            float4(prefilteredColor, 0.0);
    }
}
