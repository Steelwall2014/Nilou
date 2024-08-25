#pragma once

#include <string>
#include <utility>
#include <vector>

#include "StaticMeshResources.h"
#include "MeshBatch.h"
#include "RHIDefinitions.h"
#include "RHIResources.h"
#include "VertexFactory.h"
#include "RHICommandContext.h"

namespace nilou {

    class FMeshDrawShaderBindings
    {
    public:

        void SetDescriptorSet(uint32 SetIndex, RDGDescriptorSet* DescriptorSet) { DescriptorSets[SetIndex] = DescriptorSet; }

        std::map<uint32, RDGDescriptorSet*> DescriptorSets;

        void SetOnCommandList(RHICommandList& RHICmdList) const;
    };


    class FMeshDrawCommand
    {
    public:
    
        FMeshDrawCommand();

        #ifdef NILOU_DEBUG 
        const FVertexFactory* DebugVertexFactory;
        const FMaterialRenderProxy* DebugMaterial;
        #endif
        
        /**
        * Resource bindings
        */
	    FMeshDrawShaderBindings ShaderBindings;
        std::vector<FVertexInputStream> VertexStreams;
        RDGBuffer* IndexBuffer;

        /**
        * PSO
        */
        // FGraphicsMinimalPipelineStateId CachedPipelineId;
        FRHIPipelineState *PipelineState;

        /**
        * Draw command parameters
        */
        uint32 FirstIndex;
        uint32 NumPrimitives;
        uint32 NumInstances;

        union
        {
            struct 
            {
                uint32 BaseVertexIndex;
                uint32 NumVertices;
            } VertexParams;
            
            struct  
            {
                RHIBuffer* Buffer;
                uint32 Offset;
            } IndirectArgs;
        };

        uint8 StencilRef;

        void SubmitDraw(RHICommandList& RHICmdList) const;
    };
}