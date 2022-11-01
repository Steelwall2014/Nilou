#include "StaticMeshActor.h"

namespace nilou {

    void AStaticMeshActor::SetStaticMesh(std::shared_ptr<UStaticMesh> StaticMesh)
    {
        if (StaticMesh)
        {
            StaticMeshComponent->SetStaticMesh(StaticMesh);
        }
    }

}