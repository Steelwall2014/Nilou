#pragma once
#include "PrimitiveComponent.h"

namespace nilou {

    UCLASS()
    class UArrowComponent : public UPrimitiveComponent
    {
        GENERATE_CLASS_INFO()
    public:

        UArrowComponent(AActor *InOwner = nullptr);

        /** The size on screen to limit this arrow to (in screen space) */
        // float ScreenSize;

        /** Set to limit the screen size of this arrow */
        // bool bIsScreenSizeScaled;

        /** Updates the arrow's colour, and tells it to refresh */
        inline vec4 GetArrowColor() const { return ArrowColor; }

        /** Updates the arrow's size, and tells it to refresh */
        inline float GetArrowSize() const { return ArrowSize; }

        /** Updates the arrow's length, and tells it to refresh */
        inline float GetArrowLength() const { return ArrowLength; }

        /** Updates the arrow's colour, and tells it to refresh */
        virtual void SetArrowColor(vec4 NewColor);

        /** Updates the arrow's size, and tells it to refresh */
        virtual void SetArrowSize(float NewSize);

        /** Updates the arrow's length, and tells it to refresh */
        virtual void SetArrowLength(float NewLength);

        //~ Begin UPrimitiveComponent Interface.
        virtual FPrimitiveSceneProxy *CreateSceneProxy() override;
        //~ End UPrimitiveComponent Interface.

        //~ Begin USceneComponent Interface.
        virtual FBoundingBox CalcBounds(const FTransform &LocalToWorld) const override;
        //~ Begin USceneComponent Interface.

    protected:

        /** Color to draw arrow */
        vec4 ArrowColor;

        /** Relative size to scale drawn arrow by */
        float ArrowSize;

        /** Total length of drawn arrow including head */
        float ArrowLength;
    };

}