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

        UStaticMeshComponent(AActor *InOwner=nullptr)
            : UPrimitiveComponent(InOwner)
            , StaticMesh(nullptr)
        { 
            // Material = std::make_shared<FDefaultMaterial>();
        }

        void SetStaticMesh(UStaticMesh *StaticMesh);

        UStaticMesh *StaticMesh;
        std::vector<UMaterial *> MaterialSlots;

        /** Calculate the bounds of the component. Default behavior is a bounding box/sphere of zero size. */
        virtual FBoundingBox CalcBounds(const FTransform& LocalToWorld) const override;

        virtual FPrimitiveSceneProxy *CreateSceneProxy() override;
    };

    class FStaticMeshSceneProxy : public FPrimitiveSceneProxy
    {
    public:
        FStaticMeshSceneProxy(UStaticMeshComponent *Component);

        virtual void GetDynamicMeshElements(const std::vector<FViewSceneInfo*> &Views, uint32 VisibilityMap, FMeshElementCollector &Collector) override;
        // virtual void GetDynamicMeshElement(FMeshBatch &OutMeshBatch, const FSceneView &SceneView) override;

    private:
	    FStaticMeshRenderData* RenderData;
        std::vector<UMaterial *> MaterialSlots;
    };
}