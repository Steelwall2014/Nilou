#pragma once

#include <vector>
#include "UniformBuffer.h"
#include "ShaderBindings.h"
#include "ViewElementPDI.h"

namespace nilou {

    class FPrimitiveSceneProxy;

    struct FMeshBatchElement
    {

        RHIBuffer* PrimitiveUniformBuffer;

	    const class FVertexFactory* VertexFactory;

        const FIndexBuffer *IndexBuffer;

        RHIBuffer *IndirectArgsBuffer;

	    uint32 IndirectArgsOffset;

	    uint32 FirstIndex;

        /** When 0, IndirectArgsBuffer will be used. */
	    uint32 NumVertices;

        uint32 NumInstances;

        FMeshBatchElement()
            : IndexBuffer(nullptr)
            , VertexFactory(nullptr)
            , IndirectArgsBuffer(nullptr)
            , IndirectArgsOffset(0)
            , FirstIndex(0)
            , NumVertices(0)
            , NumInstances(1)
        {}
    };

    struct FMeshBatch
    {
        std::vector<FMeshBatchElement> Elements;

	    class FMaterialRenderProxy* MaterialRenderProxy;

        //uint32 ReverseCulling : 1;
        //uint32 bDisableBackfaceCulling : 1;
        uint32 CastShadow		: 1;	// Whether it can be used in shadow renderpasses.
	    uint32 bUseForDepthPass : 1;	// Whether it can be used in depth pass.
	    //uint32 bWireframe		: 1;
	    uint32 bSelectable      : 1;
        uint32 bEnableReflectionProbe : 1;
        
        FMeshBatch() 
            : CastShadow(true)
            , bUseForDepthPass(true)
            //, bWireframe(false)
            , bSelectable(true)
            , bEnableReflectionProbe(true)
        { }
    };

    class FMeshElementCollector
    {
    public:
        FMeshElementCollector(
            std::vector<std::vector<FMeshBatch>>& InPerViewMeshBatches, 
            std::vector<FViewElementPDI>& InPerViewPDI,
            FPrimitiveSceneProxy* InPrimitiveSceneProxy) 
            : PerViewMeshBatches(InPerViewMeshBatches)
            , PerViewPDI(InPerViewPDI)
            , PrimitiveSceneProxy(InPrimitiveSceneProxy)
        { 

        }

        void AddMesh(int32 ViewIndex, FMeshBatch &MeshBatch);

        void AddBatchedLine(int32 ViewIndex, const FBatchedLine &MeshBatch);

        std::vector<std::vector<FMeshBatch>>& PerViewMeshBatches;
        std::vector<FViewElementPDI>& PerViewPDI;

	    /** Current primitive being gathered. */
        FPrimitiveSceneProxy* PrimitiveSceneProxy;
    };

}