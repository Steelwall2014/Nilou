import RenderCore;

uniform Sampler2D baseColorTexture;
uniform Sampler2D metallicRoughnessTexture;
uniform Sampler2D emissiveTexture;
uniform float4 baseColorFactor;
uniform float3 emissiveFactor;
uniform float metallicFactor;
uniform float roughnessFactor;

class Cesium3DTilesMaterial : IMaterial
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
        float3 tangent_normal = float3(0.5, 0.5, 1.0);
        tangent_normal = normalize(tangent_normal * 2.0 - 1.0);
        return normalize(mul(tangent_normal, this.vs_out.GetTBN()));
    }
    float GetRoughness()
    {
        return 1;
    }
    float GetMetallic()
    {
        return 0;
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

