#include "D:/Nilou/src/Runtime/Framework/Common/Actor/Actor.h"
#include <UDRefl/UDRefl.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> nilou::AActor::StaticClass_ = nullptr;
const NClass *nilou::AActor::GetClass() const 
{ 
    return nilou::AActor::StaticClass(); 
}
const NClass *nilou::AActor::StaticClass()
{
    return nilou::AActor::StaticClass_.get();
}

template<>
struct TClassRegistry<nilou::AActor>
{
    TClassRegistry(const std::string& InName)
    {
        nilou::AActor::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<nilou::AActor>();
		Mngr.AddField<&nilou::AActor::ActorName>("ActorName");
		Mngr.AddBases<nilou::AActor, NObject>();
;
        nilou::AActor::StaticClass_->Type = Type_of<nilou::AActor>;
        nilou::AActor::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<nilou::AActor>);
    }

    static TClassRegistry<nilou::AActor> Dummy;
};
TClassRegistry<nilou::AActor> Dummy = TClassRegistry<nilou::AActor>("nilou::AActor");



void nilou::AActor::Serialize(FArchive& Ar)
{
    NObject::Serialize(Ar);
    if (this->bIsSerializing)
        return;
    this->bIsSerializing = true;
    nlohmann::json& Node = Ar.Node;
    Node["ClassName"] = "nilou::AActor";
    nlohmann::json &content = Node["Content"];

    {
        FArchive local_Ar(content["ActorName"], Ar);
        TStaticSerializer<decltype(this->ActorName)>::Serialize(this->ActorName, local_Ar);
    }
    this->bIsSerializing = false;
}

void nilou::AActor::Deserialize(FArchive& Ar)
{
    nlohmann::json& Node = Ar.Node;
    nlohmann::json &content = Node["Content"];

    if (content.contains("ActorName"))
    {
        FArchive local_Ar(content["ActorName"], Ar);
        TStaticSerializer<decltype(this->ActorName)>::Deserialize(this->ActorName, local_Ar);
    }
    NObject::Deserialize(Ar);
}
