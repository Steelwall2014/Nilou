#include "Actor.h"
#include "Common/Components/LineBatchComponent.h"

namespace nilou {

	UCLASS()
    class ALineBatchActor : public AActor
    {
		GENERATE_CLASS_INFO()
    public:
        ALineBatchActor() 
        { 
            LineBatchComponent = std::make_shared<ULineBatchComponent>(this); 
            LineBatchComponent->AttachToComponent(GetRootComponent());
        }

        std::shared_ptr<ULineBatchComponent> LineBatchComponent;
    };

}