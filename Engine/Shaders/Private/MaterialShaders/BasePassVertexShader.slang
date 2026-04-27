module BasePassVertexShader;
import RenderCore;
import BasePassCommon;

[shader("vertex")]
BasePassVertexShaderOutput MainVS<TVertexFactory: IVertexFactory, TMaterial: IMaterial, TVertexFactoryInput: IVertexFactoryInput>(TVertexFactoryInput input)
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
