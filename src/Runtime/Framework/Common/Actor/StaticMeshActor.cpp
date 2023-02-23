#include "StaticMeshActor.h"

namespace nilou {

    void AStaticMeshActor::SetStaticMesh(UStaticMesh *StaticMesh)
    {
        if (StaticMesh)
        {
            StaticMeshComponent->SetStaticMesh(StaticMesh);
        }
    }

}