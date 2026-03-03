implementing RenderCore;

public enum EShadingModel
{
    Unlit = 0,
    DefaultLit,
    OceanSubsurface,
    SkyAtmosphere,

    ShadingModelNum
};

public interface IMaterial
{
    __init(IVertexShaderOutput vs_out);
    float4 GetBaseColor();
    float3 GetEmissive();
    float3 GetWorldSpaceNormal();
    float GetRoughness();
    float GetMetallic();
    float3 GetWorldSpaceOffset();
    EShadingModel GetShadingModel();
};