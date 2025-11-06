#pragma once
#include "GameFramework/Actor.h"
#include "Components/LineBatchComponent.h"

namespace nilou {

    class NCLASS ALineBatchActor : public AActor
    {
		GENERATED_BODY()
    public:
        ALineBatchActor() 
        { 
            LineBatchComponent = CreateComponent<ULineBatchComponent>(this, "LineBatchComponent"); 
            LineBatchComponent->AttachToComponent(GetRootComponent());
        }

        NPROPERTY()
        ULineBatchComponent* LineBatchComponent;
    };

}