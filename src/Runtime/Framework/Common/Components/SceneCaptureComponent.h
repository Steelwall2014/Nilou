#pragma once
#include "PrimitiveComponent.h"

namespace nilou {

    UCLASS()
    class USceneCaptureComponent : public USceneComponent
    {
        GENERATE_CLASS_INFO()
    public:
        USceneCaptureComponent(AActor *InOwner = nullptr) 
            : USceneComponent(InOwner)
        { }

        uint8 bCaptureEveryFrame;

	    uint8 bCaptureOnMovement;

        std::vector<std::weak_ptr<UPrimitiveComponent>> HiddenComponents;
        
        void HideComponent(std::weak_ptr<UPrimitiveComponent> InComponent);

        void HideActorComponents(std::weak_ptr<AActor> InActor);
    };

    UCLASS()
    class USceneCaptureComponent2D : public USceneCaptureComponent
    {
        GENERATE_CLASS_INFO()
    public:
        USceneCaptureComponent2D(AActor *InOwner = nullptr) 
            : USceneCaptureComponent(InOwner)
        { }

    protected:

	    /** Camera field of view (in degrees). */
        float FOVAngle;

	    /** The desired width (in world units) of the orthographic view (ignored in Perspective mode) */
        float OrthoWidth;

        std::weak_ptr<class UTextureRenderTarget> TextureTarget;

    };

    UCLASS()
    class USceneCaptureComponentCube : public USceneCaptureComponent
    {
        GENERATE_CLASS_INFO()
    public:
        USceneCaptureComponentCube(AActor *InOwner = nullptr) 
            : USceneCaptureComponent(InOwner)
        { }
    };

}