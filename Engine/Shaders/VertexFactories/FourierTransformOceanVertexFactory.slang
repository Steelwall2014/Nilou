import "IVertexFactory.slang";

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

struct FPrimitiveShaderParameters
{
    double4x4 LocalToWorld;
    double4x4 ModelToLocal;
};
ParameterBlock<FPrimitiveShaderParameters> PrimitiveParameters;

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
        FourierTransformOceanVertexFactoryInput typedInput = Input as FourierTransformOceanVertexFactoryInput;
        this.InstanceIndex = typedInput.InstanceIndex;
    }

    float3 GetWorldPosition()
    {
        uint4 nodeLoc = NodeListBuffer[this.InstanceIndex];
        float scale = pow(2.0, nodeLoc.z);
        
        float2 offset = float2(nodeLoc.x, nodeLoc.y) * float2(LODParamsBuffer[nodeLoc.z].NodeMeterSize.x, LODParamsBuffer[nodeLoc.z].NodeMeterSize.y);
        float3 pos = this.Position;
        pos *= scale;
        pos += float3(offset, 0);
        
        double4x4 ModelToWorld = PrimitiveParameters.LocalToWorld * PrimitiveParameters.ModelToLocal;
        return float3(mul(double4(pos, 1), ModelToWorld).xyz);
    }

    float3 GetWorldNormal()
    {
        double4x4 ModelToWorld = PrimitiveParameters.LocalToWorld * PrimitiveParameters.ModelToLocal;
        float3x3 modelToWorld3x3 = float3x3(ModelToWorld);
        return mul(transpose(inverse(modelToWorld3x3)), this.Normal);
    }

    float4 GetWorldTangent()
    {
        double4x4 ModelToWorld = PrimitiveParameters.LocalToWorld * PrimitiveParameters.ModelToLocal;
        return mul(this.Tangent, float4x4(ModelToWorld));
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

