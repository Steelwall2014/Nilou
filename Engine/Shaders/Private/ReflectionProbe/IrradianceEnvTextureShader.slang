module IrradianceEnvTextureShader;

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

struct FIrradianceEnvTextureParameters
{
    SamplerCube              EnvironmentTexture;
    RWTexture2DArray<float4> IrradianceTexture;
    int                      TextureSize;
};
ParameterBlock<FIrradianceEnvTextureParameters> Params;

[shader("compute")]
[numthreads(1, 1, 1)]
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

        // Rebuild tangent basis with N as the normal
        right = cross(up, N);
        up    = cross(N, right);

        float3 irradiance  = float3(0.0);
        float  sampleDelta = 0.025;
        float  nrSamples   = 0.0;

        for (float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
        {
            for (float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
            {
                // Tangent-space direction
                float3 tangentSample = float3(
                    sin(theta) * cos(phi),
                    sin(theta) * sin(phi),
                    cos(theta)
                );
                // Tangent space to world space
                float3 sampleVec = tangentSample.x * right
                                 + tangentSample.y * up
                                 + tangentSample.z * N;

                float3 color = mytextureCubeLod(Params.EnvironmentTexture, sampleVec, 0.0).rgb;
                irradiance += color * cos(theta) * sin(theta);
                nrSamples++;
            }
        }
        irradiance = PI * irradiance * (1.0 / nrSamples);

        // Vulkan: flip Y axis
        Params.IrradianceTexture[int3(threadID.x, Params.TextureSize - 1 - threadID.y, face)] =
            float4(irradiance, 0.0);
    }
}
