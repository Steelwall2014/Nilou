module FourierTransformOceanVertexFactory;

import RenderCore;

namespace FourierTransformOceanVertexFactory {

struct FourierTransformOceanVertexFactoryInput : IVertexFactoryInput
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float4 Tangent : TANGENT;
    float4 Color : COLOR;
    float2 TexCoords : TEXCOORD0;
    uint InstanceIndex : SV_InstanceID;

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

struct OceanLODParam
{
    float2 NodeMeterSize;
    uint2 NodeSideNum;
};

struct FPrimitiveUniformShaderParameters
{
    float4x4 LocalToWorld;
    float4x4 WorldToLocal;
    float3 InvNonUniformScale;
};
ParameterBlock<FPrimitiveUniformShaderParameters> PrimitiveParameters;

StructuredBuffer<uint4> NodeListBuffer;
StructuredBuffer<OceanLODParam> LODParamsBuffer;

struct FourierTransformOceanVertexFactory : IVertexFactory
{
    float3 Position;
    float3 Normal;
    float4 Tangent;
    float4 Color;
    float2 TexCoords;
    uint InstanceIndex;

    __init(IVertexFactoryInput Input)
    {
        this.Position = Input.GetPosition();
        this.Normal = Input.GetNormal();
        this.Tangent = Input.GetTangent();
        this.Color = Input.GetColor();
        this.TexCoords = Input.GetTexCoords(0);
        Optional<FourierTransformOceanVertexFactoryInput> typedInput = Input as FourierTransformOceanVertexFactoryInput;
        this.InstanceIndex = typedInput.value.InstanceIndex;
    }

    float3 GetWorldPosition()
    {
        uint4 nodeLoc = NodeListBuffer[this.InstanceIndex];
        float scale = pow(2.0, nodeLoc.z);
        
        float2 offset = float2(nodeLoc.x, nodeLoc.y) * float2(LODParamsBuffer[nodeLoc.z].NodeMeterSize.x, LODParamsBuffer[nodeLoc.z].NodeMeterSize.y);
        float3 pos = this.Position;
        pos *= scale;
        pos += float3(offset, 0);
        
        return mul(float4(pos, 1), PrimitiveParameters.LocalToWorld).xyz;
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

}