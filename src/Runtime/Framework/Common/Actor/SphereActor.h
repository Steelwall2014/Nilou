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
            SphereComponent = std::make_shared<USphereComponent>(this); 
            SphereComponent->AttachToComponent(GetRootComponent());
        }
        
        std::shared_ptr<USphereComponent> SphereComponent;
    };

}