#pragma once

#include <vector>
#include "UniformBuffer.h"
#include "ShaderBindings.h"

namespace nilou {

    struct FMeshBatchElement
    {
        FElementShaderBindings Bindings;

        const FIndexBuffer *IndexBuffer;

        RHIBuffer *IndirectArgsBuffer;

	    uint32 IndirectArgsOffset;

	    // uint32 FirstIndex;

        /** When 0, IndirectArgsBuffer will be used. */
	    uint32 NumVertices;

        uint32 NumInstances;

        FMeshBatchElement()
            : IndexBuffer(nullptr)
            , IndirectArgsBuffer(nullptr)
            , IndirectArgsOffset(0)
            // , FirstIndex(0)
            , NumVertices(0)
            , NumInstances(1)
        {}
    };

    struct FMeshBatch
    {
        FMeshBatchElement Element;

	    class FVertexFactory* VertexFactory;

	    class FMaterial* MaterialRenderProxy;

        //uint32 ReverseCulling : 1;
        //uint32 bDisableBackfaceCulling : 1;
        uint32 CastShadow		: 1;	// Whether it can be used in shadow renderpasses.
	    uint32 bUseForDepthPass : 1;	// Whether it can be used in depth pass.
	    //uint32 bWireframe		: 1;
	    uint32 bSelectable      : 1;
        
        FMeshBatch() 
            : VertexFactory(nullptr)
            //, ReverseCulling(false)
            //, bDisableBackfaceCulling(false)
            , CastShadow(true)
            , bUseForDepthPass(true)
            //, bWireframe(false)
            , bSelectable(true)
        { }
    };

}