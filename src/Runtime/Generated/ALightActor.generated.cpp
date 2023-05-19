#include "D:/Nilou/src/Runtime/Framework/Common/Actor/LightActor.h"
#include <UDRefl/UDRefl.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> nilou::ALightActor::StaticClass_ = nullptr;
const NClass *nilou::ALightActor::GetClass() const 
{ 
    return nilou::ALightActor::StaticClass(); 
}
const NClass *nilou::ALightActor::StaticClass()
{
    return nilou::ALightActor::StaticClass_.get();
}

template<>
struct TClassRegistry<nilou::ALightActor>
{
    TClassRegistry(const std::string& InName)
    {
        nilou::ALightActor::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<nilou::ALightActor>();
		Mngr.AddBases<nilou::ALightActor, nilou::AActor>();
;
        nilou::ALightActor::StaticClass_->Type = Type_of<nilou::ALightActor>;
        nilou::ALightActor::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<nilou::ALightActor>);
    }

    static TClassRegistry<nilou::ALightActor> Dummy;
};
TClassRegistry<nilou::ALightActor> Dummy = TClassRegistry<nilou::ALightActor>("nilou::ALightActor");



void nilou::ALightActor::Serialize(FArchive& Ar)
{
    nilou::AActor::Serialize(Ar);
    if (this->bIsSerializing)
        return;
    this->bIsSerializing = true;
    nlohmann::json& Node = Ar.Node;
    Node["ClassName"] = "nilou::ALightActor";
    nlohmann::json &content = Node["Content"];

    this->bIsSerializing = false;
}

void nilou::ALightActor::Deserialize(FArchive& Ar)
{
    nlohmann::json& Node = Ar.Node;
    nlohmann::json &content = Node["Content"];

    nilou::AActor::Deserialize(Ar);
}
