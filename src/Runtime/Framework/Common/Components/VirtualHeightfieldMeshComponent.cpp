#include "VirtualHeightfieldMeshComponent.h"

namespace nilou {

    class FVirtualHeightfieldMeshSceneProxy : public FPrimitiveSceneProxy
    {
    public:
        FVirtualHeightfieldMeshSceneProxy(UVirtualHeightfieldMeshComponent *Component)
            : FPrimitiveSceneProxy(Component)
        {

        }
    };



    UVirtualHeightfieldMeshComponent::UVirtualHeightfieldMeshComponent(AActor *InOwner)
        : UPrimitiveComponent(InOwner)
    {

    }

    FBoundingBox UVirtualHeightfieldMeshComponent::CalcBounds(const FTransform& LocalToWorld) const
    {
        return FBoundingBox(vec3(0.f, 0.f, 0.f), vec3(1.f, 1.f, 1.f)).TransformBy(LocalToWorld);
    }

    FPrimitiveSceneProxy *UVirtualHeightfieldMeshComponent::CreateSceneProxy()
    {
        return nullptr;
    }


}