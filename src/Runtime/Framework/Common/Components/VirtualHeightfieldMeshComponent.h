#pragma once
#include "PrimitiveComponent.h"

namespace nilou {

    // TODO: Finish this component
    UCLASS()
    class UVirtualHeightfieldMeshComponent : public UPrimitiveComponent
    {
        GENERATE_CLASS_INFO()
    public:

        UVirtualHeightfieldMeshComponent(AActor *InOwner = nullptr);

        inline void SetMaterial(FMaterial *InMaterial)
        { 
            if (Material != InMaterial)
            {
                Material = InMaterial;
                MarkRenderStateDirty(); 
            }
        }

        inline FMaterial *GetMaterial() const { return Material; }

        //~ Begin UPrimitiveComponent Interface.
        virtual FPrimitiveSceneProxy *CreateSceneProxy() override;
        //~ End UPrimitiveComponent Interface.

        //~ Begin USceneComponent Interface.
        virtual FBoundingBox CalcBounds(const FTransform &LocalToWorld) const override;
        //~ Begin USceneComponent Interface.

    private:

        FMaterial *Material;

    };

}