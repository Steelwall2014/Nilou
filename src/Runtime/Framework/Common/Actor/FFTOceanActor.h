#pragma once
#include "Actor.h"
#include "Common/Components/FourierTransformOcean.h"

namespace nilou {

    class NCLASS AFFTOceanActor : public AActor
    {
		GENERATE_BODY()
    public:
        AFFTOceanActor() 
        { 
            OceanComponent = CreateComponent<UFourierTransformOceanComponent>(this); 
            OceanComponent->AttachToComponent(GetRootComponent());
        }



        std::shared_ptr<UFourierTransformOceanComponent> OceanComponent;
    };

}