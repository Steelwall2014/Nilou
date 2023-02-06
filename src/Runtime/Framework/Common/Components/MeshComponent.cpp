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

    void FStaticMeshSceneProxy::GetDynamicMeshElement(FMeshBatch &OutMeshBatch, const FSceneView &SceneView)
    {
        const FStaticMeshLODResources& LODModel = *RenderData->LODResources[0];
        FMeshBatch Mesh;
        Mesh.MaterialRenderProxy = MaterialRenderProxy;
        FStaticVertexFactory *VertexFactory = RenderData->VertexFactory.get();
        Mesh.VertexFactory = VertexFactory;
        Mesh.Element.IndexBuffer = &LODModel.IndexBuffer;
        Mesh.Element.NumVertices = LODModel.IndexBuffer.NumIndices;
        Mesh.MaterialRenderProxy->FillShaderBindings(Mesh.Element.Bindings);
        OutMeshBatch = Mesh;
    }
}