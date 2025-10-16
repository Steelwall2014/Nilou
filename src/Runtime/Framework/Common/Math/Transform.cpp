#include "Transform.h"
#include "CoreMinimal.h"

namespace nilou {

template<>
BEGIN_STRUCT_REGISTRY(FRotator, NullSuperClass, EClassFlags::Native | EClassFlags::Intrinsic)

    STRUCT_PROPERTY(Pitch)
    STRUCT_PROPERTY(Yaw)
    STRUCT_PROPERTY(Roll)

END_STRUCT_REGISTRY(FRotator)

template<>
BEGIN_STRUCT_REGISTRY(FTransform, NullSuperClass, EClassFlags::Native | EClassFlags::Intrinsic)

    STRUCT_PROPERTY(Rotation)
    STRUCT_PROPERTY(Translation)
    STRUCT_PROPERTY(Scale3D)

END_STRUCT_REGISTRY(FTransform)

}
