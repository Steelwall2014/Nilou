#include "GameFramework/Actor.h"
#include "Components/LightComponent.h"

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
        ULightComponent* LightComponent;
    };

}