#pragma once

#include "Common/Components/PrimitiveComponent.h"
// #include "Common/Components/PrimitiveSceneProxy.h"
#include "Common/StaticMeshResources.h"
#include "Material.h"

// #include "MeshComponent.generated.h"

namespace nilou {

    UCLASS()
    class UStaticMeshComponent : public UPrimitiveComponent
    {
        GENERATE_CLASS_INFO()
    public:

        UStaticMeshComponent(AActor *InOwner)
            : UPrimitiveComponent(InOwner)
        { 
            // Material = std::make_shared<FDefaultMaterial>();
        }

        void SetStaticMesh(std::shared_ptr<UStaticMesh> StaticMesh);

        std::shared_ptr<UStaticMesh> StaticMesh;
        std::shared_ptr<FMaterial> Material;

        /** Calculate the bounds of the component. Default behavior is a bounding box/sphere of zero size. */
        virtual FBoundingBox CalcBounds(const FTransform& LocalToWorld) const override;

        virtual FPrimitiveSceneProxy *CreateSceneProxy() override;
    };

    class FStaticMeshSceneProxy : public FPrimitiveSceneProxy
    {
    public:
        FStaticMeshSceneProxy(UStaticMeshComponent *Component);

        // virtual void GetDynamicMeshElements(const std::vector<class FSceneView *> &Views, uint32 VisibilityMap, std::vector<std::vector<FMeshBatch>> &OutPerViewMeshBatches, int32 LODIndex) override;
        virtual void GetDynamicMeshElement(FMeshBatch &OutMeshBatch, const FSceneView &SceneView) override;

    private:
	    FStaticMeshRenderData* RenderData;
        FMaterial *MaterialRenderProxy;
    };
}