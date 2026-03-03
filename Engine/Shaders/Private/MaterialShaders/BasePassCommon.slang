module BasePassCommon;
import RenderCore;

public struct BasePassVertexShaderOutput : IVertexShaderOutput 
{
    public float4 ClipPosition : SV_Position;
    public float3x3 TBN;
    public float3 RelativeWorldPosition;
    public float3 WorldNormal;
    public float3 WorldTangent;
    public float3 WorldBitangent;
    public float4 Color;
    public float2 TexCoords;

    public float3x3 GetTBN()
    {
        return TBN;
    }
    public float3 GetRelativeWorldPosition()
    {
        return RelativeWorldPosition;
    }
    public float4 GetClipPosition()
    {
        return ClipPosition;
    }
    public float3 GetWorldNormal()
    {
        return WorldNormal;
    }
    public float3 GetWorldTangent()
    {
        return WorldTangent;
    }
    public float3 GetWorldBitangent()
    {
        return WorldBitangent;
    }
    public float4 GetColor()
    {
        return Color;
    }
    public float2 GetTexCoords()
    {
        return TexCoords;
    }
};
