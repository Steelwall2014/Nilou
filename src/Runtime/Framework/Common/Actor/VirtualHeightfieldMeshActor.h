#include "Actor.h"
#include "Common/Components/VirtualHeightfieldMeshComponent.h"

namespace nilou {

	UCLASS()
    class AVirtualHeightfieldMeshActor : public AActor
    {
		GENERATE_CLASS_INFO()
    public:
        AVirtualHeightfieldMeshActor() 
        { 
            VHMComponent = std::make_shared<UVirtualHeightfieldMeshComponent>(this); 
            VHMComponent->AttachToComponent(GetRootComponent());
        }

        std::shared_ptr<UVirtualHeightfieldMeshComponent> VHMComponent;
    };

}