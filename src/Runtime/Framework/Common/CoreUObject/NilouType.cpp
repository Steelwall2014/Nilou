#include "NilouType.h"

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

}