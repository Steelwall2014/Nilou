#pragma once
#include "PrimitiveComponent.h"

namespace nilou {

    UCLASS()
    class UFourierTransformOceanComponent : public UPrimitiveComponent
    {
        GENERATE_CLASS_INFO()

    public:

        UFourierTransformOceanComponent(AActor* Owner=nullptr)
            : UPrimitiveComponent(Owner)
        {

        }
        
    };

}