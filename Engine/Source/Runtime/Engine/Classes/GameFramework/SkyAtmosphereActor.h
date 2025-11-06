#include "Actor.h"
#include "Components/SkyAtmosphereComponent.h"

namespace nilou {

    class NCLASS ASkyAtmosphereActor : public AActor
    {
		GENERATED_BODY()
    public:
        ASkyAtmosphereActor() 
        { 
            SkyAtmosphereComponent = CreateComponent<USkyAtmosphereComponent>(this, "SkyAtmosphereComponent"); 
            SkyAtmosphereComponent->AttachToComponent(GetRootComponent());
        }

        NPROPERTY()
        USkyAtmosphereComponent* SkyAtmosphereComponent;
    };

}