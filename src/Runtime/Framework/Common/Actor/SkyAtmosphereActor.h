#include "Actor.h"
#include "Common/Components/SkyAtmosphereComponent.h"

namespace nilou {

    class NCLASS ASkyAtmosphereActor : public AActor
    {
		GENERATED_BODY()
    public:
        ASkyAtmosphereActor() 
        { 
            SkyAtmosphereComponent = CreateComponent<USkyAtmosphereComponent>(this); 
            SkyAtmosphereComponent->AttachToComponent(GetRootComponent());
        }

        std::shared_ptr<USkyAtmosphereComponent> SkyAtmosphereComponent;
    };

}