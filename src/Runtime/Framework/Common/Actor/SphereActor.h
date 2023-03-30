#include "Actor.h"
#include "Common/Components/SphereComponent.h"

namespace nilou {

	UCLASS()
    class ASphereActor : public AActor
    {
		GENERATE_CLASS_INFO()
    public:
        ASphereActor() 
        { 
            SphereComponent = CreateComponent<USphereComponent>(this); 
            SphereComponent->AttachToComponent(GetRootComponent());
        }
        
        std::shared_ptr<USphereComponent> SphereComponent;
    };

}