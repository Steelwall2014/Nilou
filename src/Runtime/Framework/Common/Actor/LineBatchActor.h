#include "Actor.h"
#include "Common/Components/LineBatchComponent.h"

namespace nilou {

    class NCLASS ALineBatchActor : public AActor
    {
		GENERATE_BODY()
    public:
        ALineBatchActor() 
        { 
            LineBatchComponent = CreateComponent<ULineBatchComponent>(this); 
            LineBatchComponent->AttachToComponent(GetRootComponent());
        }

        std::shared_ptr<ULineBatchComponent> LineBatchComponent;
    };

}