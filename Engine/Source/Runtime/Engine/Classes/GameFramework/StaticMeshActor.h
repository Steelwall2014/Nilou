#pragma once
#include "Actor.h"
#include "Components/MeshComponent.h"

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
        UStaticMeshComponent* StaticMeshComponent;
    };

}