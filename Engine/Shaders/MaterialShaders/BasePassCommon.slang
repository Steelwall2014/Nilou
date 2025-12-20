import "../include/VertexShaderCommon.slang";

struct BasePassVertexShaderOutput : IVertexShaderOutput 
{
    float4 ClipPosition : SV_Position;
    float3x3 TBN;
    float3 RelativeWorldPosition;
    float3 WorldNormal;
    float3 WorldTangent;
    float3 WorldBitangent;
    float4 Color;
    float2 TexCoords;

    float3x3 GetTBN()
    {
        return TBN;
    }
    float3 GetRelativeWorldPosition()
    {
        return RelativeWorldPosition;
    }
    float4 GetClipPosition()
    {
        return ClipPosition;
    }
    float3 GetWorldNormal()
    {
        return WorldNormal;
    }
    float3 GetWorldTangent()
    {
        return WorldTangent;
    }
    float3 GetWorldBitangent()
    {
        return WorldBitangent;
    }
    float4 GetColor()
    {
        return Color;
    }
    float2 GetTexCoords()
    {
        return TexCoords;
    }
};
