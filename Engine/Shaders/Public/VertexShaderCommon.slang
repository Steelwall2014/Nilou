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

public struct FViewShaderParameters
{
    public double4 FrustumPlanes[6];
    public float4x4 RelWorldToView;
    public float4x4 ViewToClip;
    public float4x4 RelWorldToClip;
    public float4x4 ClipToView;
    public float4x4 RelClipToWorld;
    public float4x4 AbsWorldToClip;
    public float3 CameraPosition;
    public float3 CameraDirection;
    public int2 CameraResolution;
    public float CameraNearClipDist;
    public float CameraFarClipDist;
    public float CameraVerticalFieldOfView;
};

public ParameterBlock<FViewShaderParameters> ViewParameters;
