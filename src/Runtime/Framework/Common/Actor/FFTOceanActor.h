#pragma once
#include "Actor.h"
#include "Common/Components/FourierTransformOcean.h"

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
        std::shared_ptr<UFourierTransformOceanComponent> OceanComponent;
    };

}