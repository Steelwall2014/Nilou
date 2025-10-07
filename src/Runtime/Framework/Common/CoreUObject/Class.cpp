#include "Class.h"
#include "Object.h"
#include "base64.h"

namespace nilou {

NClass* NClass::Z_StaticClass = nullptr;

void FStructProperty::SerializeItem(FArchive& Ar, void* Value)
{
    Struct->SerializeProperties(Ar, Value);
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
    TArray<FProperty*> Properties = GetProperties(true);
    for (auto& Property : Properties)
    {
        void* Field = Property->ContainerPtrToValuePtrInternal(Data);
        Property->SerializeItem(Ar, Field);
    }
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

}
