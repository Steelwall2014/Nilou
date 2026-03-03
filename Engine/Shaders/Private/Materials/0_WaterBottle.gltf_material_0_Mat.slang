import RenderCore;

uniform Sampler2D baseColorTexture;
uniform Sampler2D metallicRoughnessTexture;
uniform Sampler2D emissiveTexture;
uniform Sampler2D normalTexture;
uniform float4 baseColorFactor;
uniform float3 emissiveFactor;
uniform float metallicFactor;
uniform float roughnessFactor;

class WaterBottleMaterial : IMaterial
{
    IVertexShaderOutput vs_out;
    __init(IVertexShaderOutput vs_out)
    {
        this.vs_out = vs_out;
    }
    float4 GetBaseColor()
    {
        return baseColorTexture.Sample(this.vs_out.GetTexCoords()) * baseColorFactor;
    }
    float3 GetEmissive()
    {
        return emissiveTexture.Sample(this.vs_out.GetTexCoords()).rgb * emissiveFactor;
    }
    float3 GetWorldSpaceNormal()
    {
        float3 tangent_normal = normalTexture.Sample(this.vs_out.GetTexCoords()).rgb;
        tangent_normal = normalize(tangent_normal * 2.0 - 1.0);
        return normalize(mul(tangent_normal, this.vs_out.GetTBN()));
    }
    float GetRoughness()
    {
        return metallicRoughnessTexture.Sample(this.vs_out.GetTexCoords()).g * roughnessFactor;
    }
    float GetMetallic()
    {
        return metallicRoughnessTexture.Sample(this.vs_out.GetTexCoords()).b * metallicFactor;
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

