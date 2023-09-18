#include "Actor.h"
#include "Common/Components/VirtualHeightfieldMeshComponent.h"

namespace nilou {

    class NCLASS AVirtualHeightfieldMeshActor : public AActor
    {
		GENERATED_BODY()
    public:
        AVirtualHeightfieldMeshActor() 
        { 
            VHMComponent = CreateComponent<UVirtualHeightfieldMeshComponent>(this); 
            VHMComponent->AttachToComponent(GetRootComponent());
        }

        NPROPERTY()
        std::shared_ptr<UVirtualHeightfieldMeshComponent> VHMComponent;
    };

}