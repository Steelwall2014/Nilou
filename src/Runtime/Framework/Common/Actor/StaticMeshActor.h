#include "Actor.h"
#include "Common/Components/MeshComponent.h"

namespace nilou {

	UCLASS()
    class AStaticMeshActor : public AActor
    {
		GENERATE_CLASS_INFO()
    public:
        AStaticMeshActor() 
        { 
            StaticMeshComponent = std::make_shared<UStaticMeshComponent>(this); 
            StaticMeshComponent->AttachToComponent(GetRootComponent());
        }

        void SetStaticMesh(UStaticMesh *StaticMesh);



        std::shared_ptr<UStaticMeshComponent> StaticMeshComponent;
    };

}