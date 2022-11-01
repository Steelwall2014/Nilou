#include "MeshComponent.h"
#include "Common/MeshBatch.h"

namespace nilou {

    void UStaticMeshComponent::SetStaticMesh(std::shared_ptr<UStaticMesh> InStaticMesh)
    {
        StaticMesh = InStaticMesh;
        StaticMesh->RenderData->InitResources();
        UpdateBounds();
        MarkRenderStateDirty();
    }

    FPrimitiveSceneProxy *UStaticMeshComponent::CreateSceneProxy()
    {
        if (StaticMesh == nullptr)
            return nullptr;
        if (StaticMesh->RenderData == nullptr)
            return nullptr;
        if (!StaticMesh->RenderData->IsInitialized())
            return nullptr;
        return new FStaticMeshSceneProxy(this);
    }

    FBoundingBox UStaticMeshComponent::CalcBounds(const FTransform &LocalToWorld) const
    {
        if (StaticMesh)
        {
            FBoundingBox &LocalBoundingBox = StaticMesh->LocalBoundingBox;
            return LocalBoundingBox.TransformBy(LocalToWorld);
        }
        else 
        {
            return USceneComponent::CalcBounds(LocalToWorld);
        }
    }

    FStaticMeshSceneProxy::FStaticMeshSceneProxy(UStaticMeshComponent *Component)
        : FPrimitiveSceneProxy(Component)
    {
        RenderData = Component->StaticMesh->RenderData.get();
        MaterialRenderProxy = Component->Material.get();
        Component->SceneProxy = this;
    }
    
    // void FStaticMeshSceneProxy::GetDynamicMeshElements(const std::vector<FSceneView *> &Views, uint32 VisibilityMap, std::vector<std::vector<FMeshBatch>> &OutPerViewMeshBatches, int32 LODIndex)
    // {
    //     for (int ViewIndex = 0; ViewIndex < Views.size(); ViewIndex++)
    //     {
    //         if (VisibilityMap & 1 << ViewIndex)
    //         {
    //             const FStaticMeshLODResources& LODModel = *RenderData->LODResources[LODIndex];
    //             FMeshBatch Mesh;
    //             Mesh.MaterialRenderProxy = MaterialRenderProxy;
    //             Mesh.VertexFactory = RenderData->VertexFactory.get();
    //             Mesh.Element.IndexBuffer = &LODModel.IndexBuffer;
    //             Mesh.Element.NumVertices = LODModel.IndexBuffer.NumIndices;
    //             Mesh.Element.PrimitiveUniformBufferResource = PrimitiveUniformBuffer.get();
    //             OutPerViewMeshBatches[ViewIndex].push_back(Mesh);
    //         }
    //     }
    // }

    void FStaticMeshSceneProxy::GetDynamicMeshElement(FMeshBatch &OutMeshBatch, const FSceneView &SceneView)
    {
        const FStaticMeshLODResources& LODModel = *RenderData->LODResources[0];
        FMeshBatch Mesh;
        Mesh.MaterialRenderProxy = MaterialRenderProxy;
        FStaticVertexFactory *VertexFactory = RenderData->VertexFactory.get();
        Mesh.VertexFactory = VertexFactory;
        Mesh.Element.IndexBuffer = &LODModel.IndexBuffer;
        Mesh.Element.NumVertices = LODModel.IndexBuffer.NumIndices;
        // Mesh.Element.UniformBuffers["FPrimitiveShaderParameters"] = PrimitiveUniformBuffer.get();
        Mesh.MaterialRenderProxy->CollectShaderBindings(Mesh.Element.Bindings);
        OutMeshBatch = Mesh;
    }
}