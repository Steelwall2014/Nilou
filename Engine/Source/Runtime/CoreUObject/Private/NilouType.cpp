#include "NObject/NilouType.h"
#include "Math/Transform.h"
#include "Math/BoxSphereBounds.h"

namespace nilou {

FFieldClass* FProperty::StaticClass()
{
    static FFieldClass StaticClass("Property", nullptr);
    return &StaticClass;
}

void* FProperty::ContainerPtrToValuePtrInternal(void* ContainerPtr, int32 ArrayIndex) const
{
    return (uint8*)ContainerPtr + Offset_Internal + ElementSize * ArrayIndex;
}

const void* FProperty::ContainerPtrToValuePtrInternal(const void* ContainerPtr, int32 ArrayIndex) const
{
    return (const uint8*)ContainerPtr + Offset_Internal + ElementSize * ArrayIndex;
}

IMPLEMENT_FIELD(FNumericProperty)
IMPLEMENT_FIELD(FBoolProperty)
IMPLEMENT_FIELD(FInt8Property)
IMPLEMENT_FIELD(FInt16Property)
IMPLEMENT_FIELD(FInt32Property)
IMPLEMENT_FIELD(FInt64Property)
IMPLEMENT_FIELD(FUInt8Property)
IMPLEMENT_FIELD(FUInt16Property)
IMPLEMENT_FIELD(FUInt32Property)
IMPLEMENT_FIELD(FUInt64Property)
IMPLEMENT_FIELD(FFloatProperty)
IMPLEMENT_FIELD(FDoubleProperty)
IMPLEMENT_FIELD(FStrProperty)
IMPLEMENT_FIELD(FStructProperty)
IMPLEMENT_FIELD(FArrayProperty)
IMPLEMENT_FIELD(FMapProperty)
IMPLEMENT_FIELD(FSetProperty)
IMPLEMENT_FIELD(FObjectProperty)
IMPLEMENT_FIELD(FVectorProperty)
IMPLEMENT_FIELD(FQuatProperty)
IMPLEMENT_FIELD(FEnumProperty)

template<>
struct TClassRegistry<FRotator> : public FClassRegistryBase
{
    using TClass = FRotator;
    TClassRegistry<FRotator>() : FClassRegistryBase(
        EMetaClass::Struct,
        "FRotator",
        nullptr,
        sizeof(FRotator),
        EClassFlags::Native | EClassFlags::Intrinsic,
        [](void* Memory){ new (Memory) FRotator(); })
    {
        TClass::Z_StaticClass = this->Class;
        STRUCT_PROPERTY(Pitch)
        STRUCT_PROPERTY(Yaw)
        STRUCT_PROPERTY(Roll)
    }
};
TClassRegistry<FRotator> ClassRegistry_FRotator;

template<>
struct TClassRegistry<FTransform> : public FClassRegistryBase
{
    using TClass = FTransform;
    TClassRegistry<FTransform>() : FClassRegistryBase(
        EMetaClass::Struct,
        "FTransform",
        nullptr,
        sizeof(FTransform),
        EClassFlags::Native | EClassFlags::Intrinsic,
        [](void* Memory){ new (Memory) FTransform(); })
    {
        TClass::Z_StaticClass = this->Class;
        STRUCT_PROPERTY(Rotation)
        STRUCT_PROPERTY(Translation)
        STRUCT_PROPERTY(Scale3D)
    }
};
TClassRegistry<FTransform> ClassRegistry_FTransform;

template<>
struct TClassRegistry<FBox> : public FClassRegistryBase
{
    using TClass = FBox;
    TClassRegistry<FBox>() : FClassRegistryBase(
        EMetaClass::Struct,
        "FBox",
        nullptr,
        sizeof(FBox),
        EClassFlags::Native | EClassFlags::Intrinsic,
        [](void* Memory){ new (Memory) FBox(); })
    {
        TClass::Z_StaticClass = this->Class;
        STRUCT_PROPERTY(Min)
        STRUCT_PROPERTY(Max)
    }
};
TClassRegistry<FBox> ClassRegistry_FBox;

template<>
struct TClassRegistry<FBoxSphereBounds> : public FClassRegistryBase
{
    using TClass = FBoxSphereBounds;
    TClassRegistry<FBoxSphereBounds>() : FClassRegistryBase(
        EMetaClass::Struct,
        "FBoxSphereBounds",
        nullptr,
        sizeof(FBoxSphereBounds),
        EClassFlags::Native | EClassFlags::Intrinsic,
        [](void* Memory){ new (Memory) FBoxSphereBounds(); })
    {
        TClass::Z_StaticClass = this->Class;
        STRUCT_PROPERTY(Origin)
        STRUCT_PROPERTY(BoxExtent)
        STRUCT_PROPERTY(SphereRadius)
    }
};
TClassRegistry<FBoxSphereBounds> ClassRegistry_FBoxSphereBounds;

}