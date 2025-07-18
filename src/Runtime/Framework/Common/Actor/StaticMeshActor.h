#include "Actor.h"
#include "Common/Components/MeshComponent.h"

namespace nilou {

    class NCLASS AStaticMeshActor : public AActor
    {
		GENERATED_BODY()
    public:
        AStaticMeshActor() 
        { 
            StaticMeshComponent = CreateComponent<UStaticMeshComponent>(this, "StaticMeshComponent"); 
            StaticMeshComponent->AttachToComponent(GetRootComponent());
        }

        void SetStaticMesh(UStaticMesh *StaticMesh);



        NPROPERTY()
        std::shared_ptr<UStaticMeshComponent> StaticMeshComponent;
    };

}