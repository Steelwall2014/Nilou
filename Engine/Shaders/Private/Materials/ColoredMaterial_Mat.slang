module ColoredMaterial_Mat;

import RenderCore;

struct FColoredMaterialParameters
{
    float4 BaseColor;
};
ParameterBlock<FColoredMaterialParameters> MaterialParameters;

struct ColoredMaterial : IMaterial
{
    IVertexShaderOutput vs_out;
    __init(IVertexShaderOutput vs_out)
    {
        this.vs_out = vs_out;
    }
    float4 GetBaseColor()
    {
        return MaterialParameters.BaseColor;
    }
    float3 GetEmissive()
    {
        return float3(0);
    }
    float3 GetWorldSpaceNormal()
    {
        return normalize(mul(float3(0, 0, 1), this.vs_out.GetTBN()));
    }
    float GetRoughness()
    {
        return 0.5;
    }
    float GetMetallic()
    {
        return 0.5;
    }
    float3 GetWorldSpaceOffset()
    {
        return MaterialParameters.BaseColor.xyz;
    }
    EShadingModel GetShadingModel()
    {
        return EShadingModel.DefaultLit;
    }
}