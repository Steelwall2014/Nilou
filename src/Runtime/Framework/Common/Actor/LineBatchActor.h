#include "Actor.h"
#include "Common/Components/LineBatchComponent.h"

namespace nilou {

    class NCLASS ALineBatchActor : public AActor
    {
		GENERATED_BODY()
    public:
        ALineBatchActor() 
        { 
            LineBatchComponent = CreateComponent<ULineBatchComponent>(this); 
            LineBatchComponent->AttachToComponent(GetRootComponent());
        }

        NPROPERTY()
        std::shared_ptr<ULineBatchComponent> LineBatchComponent;
    };

}