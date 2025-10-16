#include "CoreMinimal.h"
#include "Actor.h"
#include "Common/Components/SphereComponent.h"

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