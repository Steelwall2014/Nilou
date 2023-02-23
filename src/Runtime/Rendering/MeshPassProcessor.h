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
        std::vector<FRHIVertexInput> VertexAttributeBindings;
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