implementing RenderCore;

public interface IVertexShaderOutput
{
    float3x3 GetTBN();
    float3 GetRelativeWorldPosition();
    float4 GetClipPosition();
    float3 GetWorldNormal();
    float3 GetWorldTangent();
    float3 GetWorldBitangent();
    float4 GetColor();
    float2 GetTexCoords();
};

// View/primitive matrices: row-vector convention, use mul(position, matrix).
public struct FViewShaderParameters
{
    public double4 FrustumPlanes[6];
    public float4x4 RelWorldToView;
    public float4x4 ViewToClip;
    public float4x4 RelWorldToClip;
    public float4x4 ClipToView;
    public float4x4 RelClipToWorld;
    public float4x4 ScreenToRelativeWorld;
    public float4x4 AbsWorldToClip;
    public float3 CameraPosition;
    public float3 CameraDirection;
    public uint bIsOrthoProjection;
    public float2 ViewRectMin;
    public float4 ViewSizeAndInvSize;
    public int2 CameraResolution;
    public float CameraNearClipDist;
    public float CameraFarClipDist;
    public float CameraVerticalFieldOfView;
};

public struct FPrimitiveUniformShaderParameters
{
    public float4x4 LocalToWorld;
    public float4x4 WorldToLocal;
    public float3 InvNonUniformScale;
};

public ParameterBlock<FViewShaderParameters> ViewParameters;

// Matches UE Common.ush GetScreenWorldDir (ScreenToTranslatedWorld + 1e6 sky depth).
public static const float ScreenSkyRayDepth = 1000000.0;

// Matches UE Common.ush SvPositionToScreenPosition.
public float4 SvPositionToScreenPosition(float4 svPosition)
{
    float2 pixelPos = svPosition.xy - ViewParameters.ViewRectMin;
    float3 ndcPos = float3(
        (pixelPos * ViewParameters.ViewSizeAndInvSize.zw - 0.5) * float2(2.0, -2.0),
        svPosition.z);
    return float4(ndcPos, 1.0) * svPosition.w;
}

public float2 GetScreenPositionForProjectionType(float2 screenPosition, float sceneDepth)
{
    return ViewParameters.bIsOrthoProjection != 0 ? screenPosition : screenPosition * sceneDepth;
}

// Matches UE Common.ush GetScreenWorldDir.
public float3 GetScreenRelativeWorldDir(float4 svPosition)
{
    float2 screenPosition = SvPositionToScreenPosition(svPosition).xy;
    float depth = ScreenSkyRayDepth;
    float4 relWorldH = mul(
        float4(GetScreenPositionForProjectionType(screenPosition, depth), depth, 1.0),
        ViewParameters.ScreenToRelativeWorld);
    float3 relWorldPos = relWorldH.xyz / relWorldH.w;
    if (ViewParameters.bIsOrthoProjection != 0)
        return normalize(ViewParameters.CameraDirection);
    return normalize(relWorldPos);
}
