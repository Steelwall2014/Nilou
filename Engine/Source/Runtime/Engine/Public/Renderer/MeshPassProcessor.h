#pragma once

#include <string>
#include <vector>

#include "MeshBatch.h"
#include "RHIDefinitions.h"
#include "RHIResources.h"
#include "VertexFactory.h"

namespace nilou {

    class FMeshDrawShaderBindings
    {
    public:
        void SetBuffer(const std::string& Name, RDGBuffer* Buffer) { Buffers[Name] = Buffer; }
        void SetTexture(const std::string& Name, RDGTextureView* Texture) { Textures[Name] = Texture; }
        void SetPushConstant(EShaderStage Stage, uint32 Size, const void* Data) 
        { 
            PushConstants[Stage].resize(Size);
            memcpy(PushConstants[Stage].data(), Data, Size);
        }

        RDGBuffer* GetBuffer(const std::string& Name) const { return Buffers.at(Name); }
        RDGTextureView* GetTexture(const std::string& Name) const { return Textures.at(Name); }
        const std::vector<uint8>& GetPushConstant(EShaderStage Stage) const { return PushConstants.at(Stage); }
        
    private:
        std::map<std::string, RDGBuffer*> Buffers;
        std::map<std::string, RDGTextureView*> Textures;
        std::map<EShaderStage, std::vector<uint8>> PushConstants;
    };


    class FMeshDrawCommand
    {
    public:
    
        FMeshDrawCommand();

        #ifdef NILOU_DEBUG 
        const FVertexFactory* DebugVertexFactory = nullptr;
        const FMaterialRenderProxy* DebugMaterial = nullptr;
        #endif
        
        /**
        * Resource bindings
        */
        std::unordered_map<EShaderStage, std::vector<uint8>> PushConstants;
	    std::unordered_map<uint32, RDGDescriptorSet*> DescriptorSets;
        std::vector<FVertexInputStream> VertexStreams;
        RDGBuffer* IndexBuffer = nullptr;

        /**
        * PSO
        */
        // FGraphicsMinimalPipelineStateId CachedPipelineId;
        RHIGraphicsPipelineState *PipelineState = nullptr;

        /**
        * Draw command parameters
        */
        uint32 FirstIndex = 0;
        uint32 NumPrimitives = 0;
        uint32 NumInstances = 1;

        bool bUseIndirectArgs = false;
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

        uint8 StencilRef = 0;

        void SubmitDraw(RHICommandList& RHICmdList) const;
    };
}