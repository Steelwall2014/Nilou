#pragma once
#include "Actor.h"
#include "Components/FourierTransformOcean.h"

namespace nilou {

    class NCLASS AFFTOceanActor : public AActor
    {
		GENERATED_BODY()
    public:
        AFFTOceanActor() 
        { 
            OceanComponent = CreateComponent<UFourierTransformOceanComponent>(this, "OceanComponent");
        }



        NPROPERTY()
        UFourierTransformOceanComponent* OceanComponent;
    };

}