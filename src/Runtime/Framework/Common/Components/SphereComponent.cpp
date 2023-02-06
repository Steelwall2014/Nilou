#include "SphereComponent.h"
#include "Common/StaticMeshResources.h"
#include "Common/DynamicMeshResources.h"
#include "Common/PrimitiveUtils.h"
#include "Material.h"


namespace nilou {

    class FSphereSceneProxy : public FPrimitiveSceneProxy
    {
    public:
        FSphereSceneProxy(USphereComponent *Component)
            : FPrimitiveSceneProxy(Component)
            , VertexFactory("FArrowSceneProxy")
            , SphereRadius(Component->GetUnscaledSphereRadius())
            , Material(Component->GetMaterial())
        {
            std::vector<FDynamicMeshVertex> OutVerts;
            std::vector<uint32> OutIndices;
            GetSphereMesh(vec3(0), FRotator::ZeroRotator, vec3(SphereRadius), 30, 30, OutVerts, OutIndices);

            {            
                IndexBuffer.Init(OutIndices);
                VertexBuffers.InitFromDynamicVertex(&VertexFactory, OutVerts);
                BeginInitResource(&IndexBuffer);
            }
        }

        virtual void GetDynamicMeshElement(FMeshBatch &OutMeshBatch, const FSceneView &SceneView)
        {
            FMeshBatch Mesh;
            Mesh.MaterialRenderProxy = Material;
            Mesh.VertexFactory = &VertexFactory;
            Mesh.Element.IndexBuffer = &IndexBuffer;
            Mesh.Element.NumVertices = VertexBuffers.Positions.GetNumVertices();
            Mesh.MaterialRenderProxy->FillShaderBindings(Mesh.Element.Bindings);
            OutMeshBatch = Mesh;
        }

    private:
        FStaticMeshVertexBuffers VertexBuffers;
        FStaticMeshIndexBuffer IndexBuffer;
        FStaticVertexFactory VertexFactory;
        FMaterial *Material;

        float SphereRadius;

    };



    USphereComponent::USphereComponent(AActor *InOwner)
        : UPrimitiveComponent(InOwner)
        , SphereRadius(1.f)
    {

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

    FBoundingBox USphereComponent::CalcBounds(const FTransform &LocalToWorld) const
    {
	    return FBoundingBox(vec3(0), vec3(2*SphereRadius)).TransformBy(LocalToWorld);
    }


}