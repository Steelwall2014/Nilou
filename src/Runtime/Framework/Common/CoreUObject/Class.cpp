#include "Class.h"
#include "Object.h"
#include "base64.h"

namespace nilou {

NClass* NClass::Z_StaticClass = nullptr;

bool FStructProperty::Identical(const void* A, const void* B) const
{
    return Struct->IdenticalProperties(A, B);
}

void FStructProperty::SerializeItem(FArchive& Ar, void* Value)
{
    Struct->SerializeProperties(Ar, Value);
}

bool FObjectProperty::Identical(const void* A, const void* B) const
{
    return *(NObject**)A == *(NObject**)B;
}

void FObjectProperty::SerializeItem(FArchive& Ar, void* Value)
{
    nlohmann::json& Node = Ar.GetNode();
    if (Ar.IsLoading())
    {
        NObject** ppObject = (NObject**)Value;
        (*ppObject) = FindObject(Node.get<std::string>());
    }
    else 
    {
        NObject** ppObject = (NObject**)Value;
        Node = (*ppObject)->GetPathName();
    }
}

bool FEnumProperty::Identical(const void* A, const void* B) const
{
    return std::memcmp(A, B, Enum->Size) == 0;
}

void FEnumProperty::SerializeItem(FArchive& Ar, void* Value)
{
    nlohmann::json& Node = Ar.GetNode();
    if (Ar.IsLoading())
    {
        int64 value = Enum->GetValueByNameString(Node.get<std::string>());
        if (Enum->Size == 1)
        {
            *reinterpret_cast<int8*>(Value) = value;
        }
        else if (Enum->Size == 2)
        {
            *reinterpret_cast<int16*>(Value) = value;
        }
        else if (Enum->Size == 4)
        {
            *reinterpret_cast<int32*>(Value) = value;
        }
        else if (Enum->Size == 8)
        {
            *reinterpret_cast<int64*>(Value) = value;
        }
    }
    else 
    {
        int64 value = 0;
        if (Enum->Size == 1)
        {
            value = *reinterpret_cast<int8*>(Value);
        }
        else if (Enum->Size == 2)
        {
            value = *reinterpret_cast<int16*>(Value);
        }
        else if (Enum->Size == 4)
        {
            value = *reinterpret_cast<int32*>(Value);
        }
        else if (Enum->Size == 8)
        {
            value = *reinterpret_cast<int64*>(Value);
        }
        Node = Enum->GetNameStringByValue(value);
    }
}

TArray<FProperty*> NClass::GetProperties(bool bIncludeSuper) const
{
    TArray<FProperty*> OutProperties;
    if (bIncludeSuper)
    {
        NClass* SuperClass = GetSuperStruct();
        if (SuperClass)
        {
            OutProperties = SuperClass->GetProperties(bIncludeSuper);
        }
    }
    OutProperties.Append(Properties);
    return OutProperties;
}

void NClass::SerializeProperties(FArchive& Ar, void* Data) const
{
    NObject* ClassDefaultObject = GetDefaultObject();
    TArray<FProperty*> Properties = GetProperties(true);
    for (FProperty* Property : Properties)
    {
        void* Pointer = Property->ContainerPtrToValuePtrInternal(Data);
        void* PointerDefault = Property->ContainerPtrToValuePtrInternal(ClassDefaultObject);
        if (!Property->Identical(Pointer, PointerDefault))
        {
            Property->SerializeItem(Ar[Property->Name], Pointer);
        }
    }
}

bool NClass::IdenticalProperties(const void* A, const void* B) const
{
    TArray<FProperty*> Properties = GetProperties(true);
    for (FProperty* Property : Properties)
    {
        const void* PointerA = Property->ContainerPtrToValuePtrInternal(A);
        const void* PointerB = Property->ContainerPtrToValuePtrInternal(B);
        if (!Property->Identical(PointerA, PointerB))
        {
            return false;
        }
    }
    return true;
}

NObject* NClass::GetDefaultObject() const
{
    if (ClassDefaultObject == nullptr)
    {
        const_cast<NClass*>(this)->CreateDefaultObject();
    }
    return ClassDefaultObject;
}

int64 NClass::GetValueByNameString(const std::string& Name) const
{
    for (auto& Pair : EnumNames)
    {
        if (Pair.first == Name)
        {
            return Pair.second;
        }
    }
    return 0;
}

std::string NClass::GetNameStringByValue(int64 Value) const
{
    for (auto& Pair : EnumNames)
    {
        if (Pair.second == Value)
        {
            return Pair.first;
        }
    }
    return "";
}

void NClass::CreateDefaultObject()
{
    FStaticConstructObjectParameters Params;
    Params.Class = this;
    Params.Outer = GetOuter();
    Params.Name = "Default__" + GetName();
    ClassDefaultObject = StaticConstructObject_Internal(Params);
    ClassDefaultObject->SetFlags(EObjectFlags::ClassDefaultObject);
}

}
