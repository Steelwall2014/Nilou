implementing LightingPassPixelShader;

struct ShadowMappingFrustum
{
    float4x4 worldToClip;
    int2 resolution;
    float frustumFarPlane;
}
struct ShadowMappingParameters
{
    Sampler2DArray<float4> shadowMaps;
    StructuredBuffer<ShadowMappingFrustum> frustums;
    
    float CalculateVisibility(int FrustumIndex, float3 RelativePosition, float bias)
    {
        int FrustumCount = frustums.getCount();
        if (FrustumIndex == -1)
            return 0.0f;
        else if (FrustumIndex >= FrustumCount)
            return 1.0f;

        float4 LightClipNDC = float4(mul(frustums[FrustumIndex].worldToClip, float4(RelativePosition + ViewParameters.CameraPosition, 1.0)));
        LightClipNDC = ClipToNDC(LightClipNDC);
        float2 ShadowMapUV = float2(LightClipNDC.x, 1.0f - LightClipNDC.y);
        float  SceneLightSpaceDepth = LightClipNDC.z;
        float2 Resolution = float2(frustums[FrustumIndex].resolution);
        float  result = 0.0f;
        int    gridSize = 3;
        for (int i = -gridSize; i <= gridSize; i++)
        {
            for (int j = -gridSize; j <= gridSize; j++)
            {
                float2 uv_offset = float2(i, j) / Resolution;
                float2 sampleUV  = ShadowMapUV + uv_offset;

                float2 texelPos          = sampleUV * Resolution;
                float2 texelPosFraction  = frac(texelPos);

                float2 samplePosCenter = floor(texelPos);
                samplePosCenter /= Resolution;

                float4 ShadowMapDepth = float4(0.0f);// = shadowMaps.GatherRed(float3(samplePosCenter, float(FrustumIndex)));
                float4 depth = step(float4(SceneLightSpaceDepth), ShadowMapDepth + float4(bias));
                float d1 = lerp(depth.w, depth.z, texelPosFraction.x);
                float d2 = lerp(depth.x, depth.y, texelPosFraction.x);
                float d  = lerp(d1, d2, texelPosFraction.y);
                result += d;
            }
        }
        return clamp(result / float((gridSize * 2 + 1) * (gridSize * 2 + 1)), 0.0f, 1.0f);
    }

    float ShadowCalculation(float3 RelativePosition, float bias)
    {
        int FrustumCount = frustums.getCount();
        float4 ViewSpacePosition = mul(ViewParameters.RelWorldToView, float4(RelativePosition, 1.0f));
        float  depth = abs(ViewSpacePosition.z);
        if (depth > frustums[FrustumCount - 1].frustumFarPlane)
            return 1.0f;
        int  FrustumIndex = -1;
        bool bNeedsBlend  = false;
        float BlendRatio  = 0.0f;
        for (int i = FrustumCount - 1; i >= 0; i--)
        {
            float SplitLength;
            if (i > 0)
                SplitLength = frustums[i].frustumFarPlane - frustums[i - 1].frustumFarPlane;
            else
                SplitLength = frustums[0].frustumFarPlane;
            if (depth < frustums[i].frustumFarPlane)
            {
                FrustumIndex = i;
                if (frustums[i].frustumFarPlane - depth < SplitLength * 0.1f)
                {
                    bNeedsBlend = true;
                    BlendRatio  = (frustums[i].frustumFarPlane - depth) / (SplitLength * 0.1f);
                }
            }
        }
        if (bNeedsBlend)
        {
            float visibility1 = CalculateVisibility(FrustumIndex,     RelativePosition, bias);
            float visibility2 = CalculateVisibility(FrustumIndex + 1, RelativePosition, bias);
            return lerp(visibility2, visibility1, BlendRatio);
        }
        else
        {
            return CalculateVisibility(FrustumIndex, RelativePosition, bias);
        }
    }
}