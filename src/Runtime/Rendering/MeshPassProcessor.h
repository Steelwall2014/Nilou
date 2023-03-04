#pragma once

#include "Common/StaticMeshResources.h"
#include "Common/MeshBatch.h"
#include "RHIDefinitions.h"
#include "RHIResources.h"
#include "VertexFactory.h"
#include <string>
#include <utility>
#include <vector>
namespace nilou {

    class FMeshDrawShaderBindings
    {
    public:
        // FShaderParameterMapInfo ParameterMapInfo;
        std::vector<std::pair<int, RHIUniformBuffer *>> UniformBufferBindings[EPipelineStage::PipelineStageNum];
        std::vector<std::pair<int, FRHISampler *>> SamplerBindings[EPipelineStage::PipelineStageNum];
        std::vector<std::pair<int, RHIBuffer *>> BufferBindings[EPipelineStage::PipelineStageNum];
        std::vector<FRHIVertexInput> VertexAttributeBindings;

        bool SetShaderBinding(EPipelineStage Stage, const FRHIDescriptorSetLayoutBinding& Binding, FInputShaderBindings& InputBindings)
        {
            bool bResourceFound = false;
            if (Binding.ParameterType == EShaderParameterType::SPT_UniformBuffer)
            {          
                if (FUniformBuffer *UniformBuffer = 
                            InputBindings.GetElementShaderBinding<FUniformBuffer>(Binding.Name))
                {
                    UniformBufferBindings[Stage].push_back({Binding.BindingPoint, UniformBuffer->GetRHI()});
                    bResourceFound = true;
                }
                else if (RHIBuffer *Buffer = 
                            InputBindings.GetElementShaderBinding<RHIBuffer>(Binding.Name))
                {
                    BufferBindings[Stage].push_back({Binding.BindingPoint, Buffer});
                    bResourceFound = true;
                }
            }
            else if (Binding.ParameterType == EShaderParameterType::SPT_Sampler)
            {  
                if (FRHISampler *Sampler = 
                            InputBindings.GetElementShaderBinding<FRHISampler>(Binding.Name))
                {
                    SamplerBindings[Stage].push_back({Binding.BindingPoint, Sampler});
                    bResourceFound = true;
                }
            }
            return bResourceFound;
        }
    };


    class FMeshDrawCommand
    {
    public:
    
        FMeshDrawCommand();
        
        /**
        * Resource bindings
        */
        FMeshDrawShaderBindings ShaderBindings;
        // std::vector<FVertexInputStream> VertexStreams;
        
        RHIBuffer* IndexBuffer;

        /**
        * PSO
        */
        // FGraphicsMinimalPipelineStateId CachedPipelineId;
        FRHIGraphicsPipelineState *PipelineState;

        uint32 StencilRef;
        RHIDepthStencilStateRef DepthStencilState;
        RHIRasterizerStateRef RasterizerState;
        RHIBlendStateRef BlendState;

        /**
        * Draw command parameters
        */
        // uint32 FirstIndex;
        // uint32 NumPrimitives;
        bool UseIndirect;

        union
        {
            struct 
            {
                uint32 NumInstances;
                uint32 BaseVertexIndex;
                uint32 NumVertices;
            } DirectArgs;
            
            struct  
            {
                RHIBuffer* Buffer;
                uint32 Offset;
            } IndirectArgs;
        };

        void SubmitDraw(class FDynamicRHI *RHICmdList);
    };
}