#include "SphereComponent.h"
#include "StaticMeshResources.h"
#include "DynamicMeshResources.h"
#include "PrimitiveUtils.h"
#include "Material.h"


namespace nilou {

    class FSphereSceneProxy : public FPrimitiveSceneProxy
    {
    public:
        FSphereSceneProxy(USphereComponent *Component)
            : FPrimitiveSceneProxy(Component)
            , SphereRadius(Component->GetUnscaledSphereRadius())
            , Material(Component->GetMaterial())
        {
            std::vector<FDynamicMeshVertex> OutVerts;
            std::vector<uint32> OutIndices;
            GetSphereMesh(vec3(0), FRotator::ZeroRotator, vec3(SphereRadius), 30, 30, OutVerts, OutIndices);
            IndexBuffer.Init(OutIndices);
            VertexBuffers.InitFromDynamicVertex(&VertexFactory, OutVerts);
            BeginInitResource(&IndexBuffer);
            
        }

        virtual void GetDynamicMeshElements(const std::vector<FSceneView>& Views, uint32 VisibilityMap, FMeshElementCollector &Collector) override
        {
            for (int32 ViewIndex = 0; ViewIndex < Views.size(); ViewIndex++)
		    {
                if (VisibilityMap & (1 << ViewIndex))
                {
                    FMeshBatch Mesh;
                    Mesh.CastShadow = bCastShadow;
                    if (Material)
                        Mesh.MaterialRenderProxy = Material->GetRenderProxy();
                    else
                        Mesh.MaterialRenderProxy = UMaterial::GetDefaultMaterial()->GetRenderProxy();
                    Mesh.Elements[0].VertexFactory = &VertexFactory;
                    Mesh.Elements[0].IndexBuffer = &IndexBuffer;
                    Mesh.Elements[0].NumVertices = VertexBuffers.Positions.GetNumVertices();
                    Collector.AddMesh(ViewIndex, Mesh);
                }
            }
        }

        virtual void DestroyRenderThreadResources() override
        {
            FPrimitiveSceneProxy::DestroyRenderThreadResources();
            VertexBuffers.ReleaseResource();
            IndexBuffer.ReleaseResource();
        }

    private:
        FStaticMeshVertexBuffers VertexBuffers;
        FStaticMeshIndexBuffer IndexBuffer;
        FStaticVertexFactory VertexFactory;
        UMaterial *Material;

        float SphereRadius;

    };



    USphereComponent::USphereComponent()
        : SphereRadius(1.f)
    {
        Material = UMaterial::GetDefaultMaterial();
    }

    void USphereComponent::SetSphereRadius(float InSphereRadius)
    {
        SphereRadius = InSphereRadius;
        UpdateBounds();
        MarkRenderStateDirty();
    }

    
    FPrimitiveSceneProxy* USphereComponent::CreateSceneProxy()
    {
        return new FSphereSceneProxy(this);
    }

    FBoxSphereBounds USphereComponent::CalcBounds(const FTransform &LocalToWorld) const
    {
	    return FBoxSphereBounds(FBox(-vec3(SphereRadius), vec3(SphereRadius)).TransformBy(LocalToWorld));
    }


}