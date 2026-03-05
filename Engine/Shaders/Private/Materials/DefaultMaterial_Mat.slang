module DefaultMaterial_Mat;

import RenderCore;

struct FTestStructure
{
    float arr[3];
}

struct FDefaultMaterialParameters
{
    Sampler2D baseColorTexture;
    Sampler2D metallicRoughnessTexture;
    Sampler2D emissiveTexture;
    Sampler2D normalTexture;
    float4 baseColorFactor;
    float3 emissiveFactor;
    float metallicFactor;
    float roughnessFactor;
    StructuredBuffer<FTestStructure> test;
};
ParameterBlock<FDefaultMaterialParameters> MaterialParameters;

struct DefaultMaterial : IMaterial
{
    IVertexShaderOutput vs_out;
    __init(IVertexShaderOutput vs_out)
    {
        this.vs_out = vs_out;
    }
    float4 GetBaseColor()
    {
        return MaterialParameters.baseColorTexture.Sample(this.vs_out.GetTexCoords()) * MaterialParameters.baseColorFactor;
    }
    float3 GetEmissive()
    {
        return MaterialParameters.emissiveTexture.Sample(this.vs_out.GetTexCoords()).rgb * MaterialParameters.emissiveFactor;
    }
    float3 GetWorldSpaceNormal()
    {
        float3 tangent_normal = MaterialParameters.normalTexture.Sample(this.vs_out.GetTexCoords()).rgb;
        tangent_normal = normalize(tangent_normal * 2.0 - 1.0);
        return normalize(mul(tangent_normal, this.vs_out.GetTBN()));
    }
    float GetRoughness()
    {
        return MaterialParameters.metallicRoughnessTexture.Sample(this.vs_out.GetTexCoords()).g * MaterialParameters.roughnessFactor;
    }
    float GetMetallic()
    {
        return MaterialParameters.metallicRoughnessTexture.Sample(this.vs_out.GetTexCoords()).b * MaterialParameters.metallicFactor;
    }
    float3 GetWorldSpaceOffset()
    {
        return ViewParameters.CameraDirection;
    }
    EShadingModel GetShadingModel()
    {
        return EShadingModel.DefaultLit;
    }
};