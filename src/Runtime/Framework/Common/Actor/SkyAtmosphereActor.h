#include "Actor.h"
#include "Common/Components/SkyAtmosphereComponent.h"

namespace nilou {

	UCLASS()
    class ASkyAtmosphereActor : public AActor
    {
		GENERATE_CLASS_INFO()
    public:
        ASkyAtmosphereActor() 
        { 
            SkyAtmosphereComponent = std::make_shared<USkyAtmosphereComponent>(this); 
            SkyAtmosphereComponent->AttachToComponent(GetRootComponent());
        }

        std::shared_ptr<USkyAtmosphereComponent> SkyAtmosphereComponent;
    };

}