#include "Transform.h"
#include "Common/CoreUObject/Class.h"

namespace nilou {

template<> NClass* FRotator::Z_StaticClass = nullptr;
BEGIN_CLASS_REGISTRY(Struct, FRotator, NullSuperClass, EClassFlags::Native | EClassFlags::Intrinsic)

    CLASS_PROPERTY(Pitch)
    CLASS_PROPERTY(Yaw)
    CLASS_PROPERTY(Roll)

END_CLASS_REGISTRY(FRotator)

template<> NClass* FTransform::Z_StaticClass = nullptr;
BEGIN_CLASS_REGISTRY(Struct, FTransform, NullSuperClass, EClassFlags::Native | EClassFlags::Intrinsic)

    CLASS_PROPERTY(Rotation)
    CLASS_PROPERTY(Translation)
    CLASS_PROPERTY(Scale3D)

END_CLASS_REGISTRY(FTransform)

}
