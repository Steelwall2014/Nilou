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
        void SetBuffer(const std::string& Name, RDGBuffer* Buffer) { Buffers[Name] = Buffer; }
        void SetTexture(const std::string& Name, RDGTextureView* Texture) { Textures[Name] = Texture; }

        RDGBuffer* GetBuffer(const std::string& Name) const { return Buffers.at(Name); }
        RDGTextureView* GetTexture(const std::string& Name) const { return Textures.at(Name); }
        
    private:
        std::map<std::string, RDGBuffer*> Buffers;
        std::map<std::string, RDGTextureView*> Textures;
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
	    std::unordered_map<uint32, RDGDescriptorSet*> DescriptorSets;
        std::vector<FVertexInputStream> VertexStreams;
        RDGBuffer* IndexBuffer;

        /**
        * PSO
        */
        // FGraphicsMinimalPipelineStateId CachedPipelineId;
        RHIGraphicsPipelineState *PipelineState;

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