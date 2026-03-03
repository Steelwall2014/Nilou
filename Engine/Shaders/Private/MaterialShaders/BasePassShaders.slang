module BasePassShaders;
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

struct FBasePassPixelShaderParameters
{
    SamplerCube IrradianceTexture;
    SamplerCube PrefilteredTexture;
    Sampler2D IBL_BRDF_LUT;
}
ParameterBlock<FBasePassPixelShaderParameters> Params;

struct BasePassPixelShaderOutput
{
    float3 SceneColor;
    float4 BaseColor;
    float3 RelativeWorldSpacePosition;
    float3 WorldSpaceNormal;
    float2 MetallicRoughness;
    float3 Emissive;
    EShadingModel ShadingModel;
};

float getMipLevelFromRoughness(float roughness)
{
    return roughness;// * (PrefilterEnvTextureNumMips-1);
}
float3 fresnelSchlickRoughness(float cosTheta, float3 F0, float roughness)
{
    return F0 + (max(float3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
} 

float3 CalcIndirectLighting(IVertexShaderOutput vs_out, float3 baseColor, float metallic, float roughness)
{
    float3 V = normalize(-vs_out.GetRelativeWorldPosition());
    float3 N = normalize(vs_out.GetWorldNormal());
    if (dot(N, V) < 0)
        N = -N;
    float NdotV = max(dot(N, V), 0);
    float3 R = reflect(-V, N);
    float lod = getMipLevelFromRoughness(roughness);

    float3 irradiance = mytextureCube(Params.IrradianceTexture, R).rgb;
    float3 prefilteredColor = mytextureCubeLod(Params.PrefilteredTexture, R, lod).rgb;
    float2 envBRDF = Params.IBL_BRDF_LUT.Sample(float2(NdotV, roughness)).rg;

    float3 F0 = float3(0.04); 
    F0 = lerp(F0, baseColor, metallic);
    float3 F = fresnelSchlickRoughness(NdotV, F0, roughness);
    float3 kS = F;
    float3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;  

    float3 diffuse = irradiance * baseColor;
    float3 specular = prefilteredColor * (F * envBRDF.x + envBRDF.y);
    return kD * diffuse + specular;
}

[shader("pixel")]
BasePassPixelShaderOutput MainPS<TMaterial : IMaterial>(BasePassVertexShaderOutput vs_out) 
{
    TMaterial material = TMaterial(vs_out);
    BasePassPixelShaderOutput output;
    output.WorldSpaceNormal = material.GetWorldSpaceNormal();
//    vec3 TangentSpaceNormal = MaterialGetTangentSpaceNormal(vs_out).rgb;
//    TangentSpaceNormal = normalize(TangentSpaceNormal * 2.0f - 1.0f);   
//    WorldSpaceNormal = normalize(vs_out.TBN * TangentSpaceNormal);
    float4 GammaBaseColor = material.GetBaseColor();
    output.BaseColor = float4(GammaToLinear(GammaBaseColor.rgb), GammaBaseColor.a);
    output.RelativeWorldSpacePosition = vs_out.GetRelativeWorldPosition();
    output.MetallicRoughness.x = material.GetMetallic();
    output.MetallicRoughness.y = material.GetRoughness();
    output.Emissive = GammaToLinear(material.GetEmissive());
    output.ShadingModel = material.GetShadingModel();
    output.SceneColor = CalcIndirectLighting(vs_out, output.BaseColor.rgb, output.MetallicRoughness.x, output.MetallicRoughness.y);
    return output;
}