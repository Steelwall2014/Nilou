interface IVertexShaderOutput
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

struct FViewShaderParameters
{
    double4 FrustumPlanes[6];
    float4x4 RelWorldToView;
    float4x4 ViewToClip;
    float4x4 RelWorldToClip;
    float4x4 ClipToView;
    float4x4 RelClipToWorld;
    float4x4 AbsWorldToClip;
    float3 CameraPosition;
    float3 CameraDirection;
    int2 CameraResolution;
    float CameraNearClipDist;
    float CameraFarClipDist;
    float CameraVerticalFieldOfView;
};
