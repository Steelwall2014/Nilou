#include "Actor.h"
#include "Common/Components/MeshComponent.h"

namespace nilou {

    class NCLASS AStaticMeshActor : public AActor
    {
		GENERATE_BODY()
    public:
        AStaticMeshActor() 
        { 
            StaticMeshComponent = CreateComponent<UStaticMeshComponent>(this); 
            StaticMeshComponent->AttachToComponent(GetRootComponent());
        }

        void SetStaticMesh(UStaticMesh *StaticMesh);



        std::shared_ptr<UStaticMeshComponent> StaticMeshComponent;
    };

}