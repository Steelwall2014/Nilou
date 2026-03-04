module StaticMeshVertexFactory;

import RenderCore;

struct StaticMeshVertexFactoryInput : IVertexFactoryInput
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float4 Tangent : TANGENT;
    float4 Color : COLOR;
    float2 TexCoords : TEXCOORD0;

    float3 GetPosition()
    {
        return this.Position;
    }
    float3 GetNormal()
    {
        return this.Normal;
    }
    float4 GetTangent()
    {
        return this.Tangent;
    }
    float4 GetColor()
    {
        return this.Color;
    }
    float2 GetTexCoords(int index)
    {
        return this.TexCoords;
    }
}

ParameterBlock<FPrimitiveUniformShaderParameters> PrimitiveParameters;

struct StaticMeshVertexFactory : IVertexFactory
{
    float3 Position;
    float3 Normal;
    float4 Tangent;
    float4 Color;
    float2 TexCoords;

    __init(IVertexFactoryInput Input)
    {
        this.Position = Input.GetPosition();
        this.Normal = Input.GetNormal();
        this.Tangent = Input.GetTangent();
        this.Color = Input.GetColor();
        this.TexCoords = Input.GetTexCoords(0);
    }
    float3 GetWorldPosition()
    {
        return mul(float4(this.Position, 1), PrimitiveParameters.LocalToWorld).xyz;
    }
    float3 GetWorldNormal()
    {
        return RotateLocalToWorld(this.Normal, PrimitiveParameters.LocalToWorld, PrimitiveParameters.InvNonUniformScale);
    }
    float4 GetWorldTangent()
    {
        return mul(this.Tangent, PrimitiveParameters.LocalToWorld);
    }
    float4 GetColor()
    {
        return this.Color;
    }
    float2 GetTexCoords(int index)
    {
        return this.TexCoords;
    }
}
