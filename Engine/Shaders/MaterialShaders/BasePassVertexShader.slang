import "../include/VertexShaderCommon.slang";
import "../Materials/IMaterial.slang";
import "../VertexFactories/IVertexFactory.slang";

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

ParameterBlock<FViewShaderParameters> ViewParameters;

[shader("vertex")]
BasePassVertexShaderOutput Main<TVertexFactory: IVertexFactory, TMaterial: IMaterial, TVertexFactoryInput: IVertexFactoryInput>(TVertexFactoryInput input)
{
    TVertexFactory vertexFactory = TVertexFactory(input);
    BasePassVertexShaderOutput vs_out = BasePassVertexShaderOutput();
    vs_out.WorldNormal = vertexFactory.GetWorldNormal();
    float4 tangent = vertexFactory.GetWorldTangent();
    float3 bitangent = normalize(cross(vs_out.WorldNormal, tangent.xyz)) * tangent.w;
    vs_out.TBN = float3x3(tangent.xyz, bitangent, vs_out.WorldNormal);
    vs_out.Color = vertexFactory.GetColor();
    vs_out.TexCoords = vertexFactory.GetTexCoords(0);
    float3 worldPosition = vertexFactory.GetWorldPosition();
    vs_out.RelativeWorldPosition = worldPosition - ViewParameters.CameraPosition;
    TMaterial material = TMaterial(vs_out);
    vs_out.RelativeWorldPosition += material.GetWorldSpaceOffset();
    vs_out.ClipPosition = mul(float4(vs_out.RelativeWorldPosition, 1), ViewParameters.RelWorldToClip);
    return vs_out;
}
