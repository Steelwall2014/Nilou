import "IMaterial.slang";
import "../include/VertexShaderCommon.slang";

struct FMaterialParameters
{
    Sampler2D baseColorTexture;
    Sampler2D metallicRoughnessTexture;
    Sampler2D emissiveTexture;
    Sampler2D normalTexture;
    float4 baseColorFactor;
    float3 emissiveFactor;
    float metallicFactor;
    float roughnessFactor;
};
ParameterBlock<FMaterialParameters> MaterialParams;

struct DefaultMaterial : IMaterial
{
    IVertexShaderOutput vs_out;
    __init(IVertexShaderOutput vs_out)
    {
        this.vs_out = vs_out;
    }
    float4 GetBaseColor()
    {
        return MaterialParams.baseColorTexture.Sample(this.vs_out.GetTexCoords()) * MaterialParams.baseColorFactor;
    }
    float3 GetEmissive()
    {
        return MaterialParams.emissiveTexture.Sample(this.vs_out.GetTexCoords()).rgb * MaterialParams.emissiveFactor;
    }
    float3 GetWorldSpaceNormal()
    {
        float3 tangent_normal = MaterialParams.normalTexture.Sample(this.vs_out.GetTexCoords()).rgb;
        tangent_normal = normalize(tangent_normal * 2.0 - 1.0);
        return normalize(mul(tangent_normal, this.vs_out.GetTBN()));
    }
    float GetRoughness()
    {
        return MaterialParams.metallicRoughnessTexture.Sample(this.vs_out.GetTexCoords()).g * MaterialParams.roughnessFactor;
    }
    float GetMetallic()
    {
        return MaterialParams.metallicRoughnessTexture.Sample(this.vs_out.GetTexCoords()).b * MaterialParams.metallicFactor;
    }
    float3 GetWorldSpaceOffset()
    {
        return float3(0);
    }
    EShadingModel GetShadingModel()
    {
        return EShadingModel.DefaultLit;
    }
};