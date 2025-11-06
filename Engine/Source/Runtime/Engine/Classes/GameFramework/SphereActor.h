#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"

namespace nilou {

    class NCLASS ASphereActor : public AActor
    {
		GENERATED_BODY()
    public:
        ASphereActor() 
        { 
            SphereComponent = CreateComponent<USphereComponent>(this, "SphereComponent"); 
            SphereComponent->AttachToComponent(GetRootComponent());
        }
        
        NPROPERTY()
        USphereComponent* SphereComponent;
    };

}