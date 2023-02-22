#pragma once
#include "PrimitiveComponent.h"
#include "Material.h"

namespace nilou {

    UCLASS()
    class USphereComponent : public UPrimitiveComponent
    {
        GENERATE_CLASS_INFO()
    public:

        USphereComponent(AActor *InOwner = nullptr);

        /**
        * Change the sphere radius. This is the unscaled radius, before component scale is applied.
        * @param	InSphereRadius: the new sphere radius
        */
        inline void SetSphereRadius(float InSphereRadius);

        // @return the radius of the sphere, with component scale applied.
        inline float GetScaledSphereRadius() const 
        { 
            return SphereRadius * GetShapeScale(); 
        }

        // @return the radius of the sphere, ignoring component scale.
        inline float GetUnscaledSphereRadius() const 
        { 
            return SphereRadius; 
        }

        // Get the scale used by this shape. This is a uniform scale that is the minimum of any non-uniform scaling.
        // @return the scale used by this shape.
        inline float USphereComponent::GetShapeScale() const
        {
            return GetComponentTransform().GetMinimumAxisScale();
        }

        inline void SetMaterial(UMaterial *InMaterial) 
        { 
            if (Material != InMaterial)
            {
                Material = InMaterial;
                MarkRenderStateDirty(); 
            }
        }

        inline UMaterial *GetMaterial() const { return Material; }

        //~ Begin UPrimitiveComponent Interface.
        virtual FPrimitiveSceneProxy *CreateSceneProxy() override;
        //~ End UPrimitiveComponent Interface.

        //~ Begin USceneComponent Interface.
        virtual FBoundingBox CalcBounds(const FTransform &LocalToWorld) const override;
        //~ Begin USceneComponent Interface.

    private:

        /** The radius of the sphere**/
        float SphereRadius;

        UMaterial *Material;

    };

}