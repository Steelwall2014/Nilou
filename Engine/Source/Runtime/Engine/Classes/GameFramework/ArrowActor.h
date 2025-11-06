#pragma once
#include "GameFramework/Actor.h"
#include "Components/ArrowComponent.h"

namespace nilou {

    class NCLASS AArrowActor : public AActor
    {
		GENERATED_BODY()
    public:
        AArrowActor();
		
    protected:

        NPROPERTY()
        UArrowComponent* xArrowComponent;

        NPROPERTY()
        UArrowComponent* yArrowComponent;

        NPROPERTY()
        UArrowComponent* zArrowComponent;
    };

}