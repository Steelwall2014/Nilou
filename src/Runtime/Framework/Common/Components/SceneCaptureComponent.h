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

}