#include "D:/Nilou/src/Runtime/RHI/RHI.h"
#include <UDRefl/UDRefl.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> NObject::StaticClass_ = nullptr;
const NClass *NObject::GetClass() const 
{ 
    return NObject::StaticClass(); 
}
const NClass *NObject::StaticClass()
{
    return NObject::StaticClass_.get();
}

template<>
struct TClassRegistry<NObject>
{
    TClassRegistry(const std::string& InName)
    {
        NObject::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<NObject>();
		Mngr.AddMethod<&NObject::GetClassName>("GetClassName");
		Mngr.AddMethod<&NObject::IsA>("IsA");
;
        NObject::StaticClass_->Type = Type_of<NObject>;
        NObject::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<NObject>);
    }

    static TClassRegistry<NObject> Dummy;
};
TClassRegistry<NObject> Dummy = TClassRegistry<NObject>("NObject");



void NObject::Serialize(FArchive& Ar)
{
    
    if (this->bIsSerializing)
        return;
    this->bIsSerializing = true;
    nlohmann::json& Node = Ar.Node;
    Node["ClassName"] = "NObject";
    nlohmann::json &content = Node["Content"];

    this->bIsSerializing = false;
}

void NObject::Deserialize(FArchive& Ar)
{
    nlohmann::json& Node = Ar.Node;
    nlohmann::json &content = Node["Content"];

    
}
