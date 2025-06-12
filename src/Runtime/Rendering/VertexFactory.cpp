#include "VertexFactory.h"
#include "Common/Asset/AssetLoader.h"
#include "DynamicRHI.h"
#include "Templates/ObjectMacros.h"

namespace nilou {
    
    std::vector<FVertexInputStream> FVertexFactory::GetVertexInputStreams() const
    {
        std::vector<FVertexInputStream> Out;
        for (int StreamIndex = 0; StreamIndex < Streams.size(); StreamIndex++)
        {
            FVertexInputStream VertexStream;
            VertexStream.StreamIndex = StreamIndex;
            VertexStream.Offset = Streams[StreamIndex].Offset;
            VertexStream.VertexBuffer = Streams[StreamIndex].VertexBuffer->VertexBufferRDG;
            Out.push_back(VertexStream);
        }
        return Out;
    }

    // void FVertexFactory::FillShaderBindings(
    //     std::map<std::string, RHIUniformBuffer *> &OutUniformBufferBindings, 
    //     std::map<std::string, FRHISampler *> &OutSamplerBindings, 
    //     std::vector<FRHIVertexInput *> &OutVertexInputs)
    // {
    //     for (FRHIVertexInput *VertexInput : VertexInputList)
    //         OutVertexInputs.push_back(VertexInput);

    //     for (auto [ParamName, UniformBuffer] : UniformBuffers)
    //     {
    //         OutUniformBufferBindings[ParamName] = UniformBuffer->GetRHI();
    //     }

    //     for (auto [ParamName, Sampler] : Samplers)
    //     {
    //         OutSamplerBindings[ParamName] = Sampler;
    //     }
    // }

    // void FVertexFactory::InitDeclaration(const FVertexDeclarationElementList &ElementList)  
    // {
    //     Declaration = FDynamicRHI::Get()->RHICreateVertexDeclaration(ElementList);
    // }

    // FVertexElement FVertexFactory::AccessStreamComponent(const FVertexStreamComponent& Component, uint8 AttributeIndex) const
    // {
    //     return FVertexElement(Component.Offset,Component.Type,AttributeIndex,Component.Stride);
    // }

    /*==================Implement FVertexFactory=========================*/
    IMPLEMENT_VERTEX_FACTORY_TYPE(FVertexFactory, "")
    // FVertexFactoryType FVertexFactory::StaticType("FVertexFactory", "");
    // FVertexFactoryType *FVertexFactory::GetType() const { return &StaticType; }
    /*==================Implement FVertexFactory=========================*/
}