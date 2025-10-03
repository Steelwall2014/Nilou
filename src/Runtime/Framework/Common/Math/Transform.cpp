#include "Transform.h"
#include "Common/CoreUObject/Class.h"

namespace nilou {

std::unique_ptr<NClass> FRotator::Z_StaticClass = nullptr;
NClass *FRotator::GetClass() const 
{ 
    return FRotator::StaticClass(); 
}
NClass *FRotator::StaticClass()
{
    return FRotator::Z_StaticClass.get();
}
BEGIN_CLASS_REGISTRY(Struct, FRotator, SerializePrivate::NullSuperClass, EClassFlags::Native | EClassFlags::Intrinsic)

    CLASS_PROPERTY(Pitch)
    CLASS_PROPERTY(Yaw)
    CLASS_PROPERTY(Roll)

END_CLASS_REGISTRY(FRotator)

std::unique_ptr<NClass> FTransform::Z_StaticClass = nullptr;
NClass *FTransform::GetClass() const 
{ 
    return FTransform::StaticClass(); 
}
NClass *FTransform::StaticClass()
{
    return FTransform::Z_StaticClass.get();
}
BEGIN_CLASS_REGISTRY(Struct, FTransform, SerializePrivate::NullSuperClass, EClassFlags::Native | EClassFlags::Intrinsic)

    CLASS_PROPERTY(Rotation)
    CLASS_PROPERTY(Translation)
    CLASS_PROPERTY(Scale3D)

END_CLASS_REGISTRY(FTransform)

}
