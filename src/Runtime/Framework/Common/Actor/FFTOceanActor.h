#pragma once
#include "Actor.h"
#include "Common/Components/FourierTransformOcean.h"

namespace nilou {

	UCLASS()
    class AFFTOceanActor : public AActor
    {
		GENERATE_CLASS_INFO()
    public:
        AFFTOceanActor() 
        { 
            OceanComponent = std::make_shared<UFourierTransformOceanComponent>(this); 
            OceanComponent->AttachToComponent(GetRootComponent());
        }



        std::shared_ptr<UFourierTransformOceanComponent> OceanComponent;
    };

}