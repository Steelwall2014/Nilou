#include "Actor.h"
#include "Common/Components/SphereComponent.h"

namespace nilou {

    class NCLASS ASphereActor : public AActor
    {
		GENERATE_BODY()
    public:
        ASphereActor() 
        { 
            SphereComponent = CreateComponent<USphereComponent>(this); 
            SphereComponent->AttachToComponent(GetRootComponent());
        }
        
        std::shared_ptr<USphereComponent> SphereComponent;
    };

}