#include "MeshComponent.h"
#include "MeshBatch.h"

namespace nilou {

    void UStaticMeshComponent::SetStaticMesh(UStaticMesh *InStaticMesh)
    {
        StaticMesh = InStaticMesh;
        StaticMesh->RenderData->InitResources();
        MaterialSlots = InStaticMesh->MaterialSlots;
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
        MaterialSlots = Component->MaterialSlots;
        Component->SceneProxy = this;
    }

    void FStaticMeshSceneProxy::GetDynamicMeshElements(const std::vector<FSceneView> &Views, uint32 VisibilityMap, FMeshElementCollector &Collector)
    {
        for (int32 ViewIndex = 0; ViewIndex < Views.size(); ViewIndex++)
		{
			if (VisibilityMap & (1 << ViewIndex))
			{
                const FStaticMeshLODResources& LODModel = *RenderData->LODResources[0];
                for (int SectionIndex = 0; SectionIndex < LODModel.Sections.size(); SectionIndex++)
                {
                    const FStaticMeshSection &Section = *LODModel.Sections[SectionIndex].get();
                    FMeshBatch Mesh;
                    Mesh.CastShadow = Section.bCastShadow;
                    Mesh.Element.VertexFactory = &Section.VertexFactory;
                    Mesh.Element.IndexBuffer = &Section.IndexBuffer;
                    Mesh.Element.NumVertices = Section.GetNumVertices();
                    Mesh.MaterialRenderProxy = MaterialSlots[Section.MaterialIndex]->GetResource()->CreateRenderProxy();
                    Mesh.Element.Bindings.SetElementShaderBinding("FPrimitiveShaderParameters", PrimitiveUniformBuffer->GetRHI());
                    Collector.AddMesh(ViewIndex, Mesh);
                }
			}
		}
    }
}