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
    // class FMaterialParameterMapInfo
    // {
    // public:
    //     std::vector<FShaderParameterInfo> UniformBuffers;
    //     std::vector<FShaderParameterInfo> TextureSamplers;
    // };

    // class FMaterialShaderBindings
    // {
    // public:
    //     FMaterialParameterMapInfo ParameterMapInfo;
    //     std::vector<RHIUniformBuffer *> UniformBufferBindings;
    //     std::vector<FRHISamplerState *> SamplerBindings;
    // };


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

    // class FMeshPassProcessor
    // {
    // public:
    //     virtual void AddMeshBatch(const FMeshBatch &Mesh, std::vector<FMeshDrawCommand> *OutDrawCommandList) = 0;

    // };

    // class FBasePassMeshProcessor : public FMeshPassProcessor
    // {
    // public:
    //     virtual void AddMeshBatch(const FMeshBatch &Mesh, std::vector<FMeshDrawCommand> *OutDrawCommandList) override;
    // };
}