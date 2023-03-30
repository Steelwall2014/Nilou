#include "Actor.h"
#include "Common/Components/LightComponent.h"

namespace nilou {

	UCLASS()
    class ALightActor : public AActor
    {
		GENERATE_CLASS_INFO()
    public:
        ALightActor() 
        { 
            LightComponent = CreateComponent<ULightComponent>(this); 
            LightComponent->AttachToComponent(GetRootComponent());
        }



        std::shared_ptr<ULightComponent> LightComponent;
    };

}