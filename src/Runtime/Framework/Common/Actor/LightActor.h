#include "Actor.h"
#include "Common/Components/LightComponent.h"

namespace nilou {

    class NCLASS ALightActor : public AActor
    {
		GENERATED_BODY()
    public:
        ALightActor() 
        { 
            LightComponent = CreateComponent<ULightComponent>(this, "LightComponent"); 
            LightComponent->AttachToComponent(GetRootComponent());
        }



        NPROPERTY()
        std::shared_ptr<ULightComponent> LightComponent;
    };

}