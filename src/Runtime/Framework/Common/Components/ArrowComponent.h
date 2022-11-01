#pragma once
#include "PrimitiveComponent.h"

namespace nilou {

    UCLASS()
    class UArrowComponent : public UPrimitiveComponent
    {
        GENERATE_CLASS_INFO()
    public:

        UArrowComponent(AActor *InOwner);

        /** Color to draw arrow */
        vec4 ArrowColor;

        /** Relative size to scale drawn arrow by */
        float ArrowSize;

        /** Total length of drawn arrow including head */
        float ArrowLength;

        /** The size on screen to limit this arrow to (in screen space) */
        float ScreenSize;

        /** Set to limit the screen size of this arrow */
        bool bIsScreenSizeScaled;

        /** Updates the arrow's colour, and tells it to refresh */
        virtual void SetArrowColor(vec4 NewColor);

        //~ Begin UPrimitiveComponent Interface.
        virtual FPrimitiveSceneProxy *CreateSceneProxy() override;
        //~ End UPrimitiveComponent Interface.

        //~ Begin USceneComponent Interface.
        virtual FBoundingBox CalcBounds(const FTransform &LocalToWorld) const override;
        //~ Begin USceneComponent Interface.
    };

}