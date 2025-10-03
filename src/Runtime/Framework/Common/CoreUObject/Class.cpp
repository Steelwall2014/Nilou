#include "Class.h"
#include "Object.h"
#include "base64.h"

namespace nilou {

std::unique_ptr<NClass> NClass::Z_StaticClass = nullptr;
NClass *NClass::GetClass() const 
{ 
    return NClass::StaticClass(); 
}
NClass *NClass::StaticClass()
{
    return NClass::Z_StaticClass.get();
}

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

}
