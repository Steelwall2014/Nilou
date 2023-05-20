#include "D:/Nilou/src/Runtime/Framework/Common/Actor/SkyAtmosphereActor.h"
#include <UDRefl/UDRefl.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> nilou::UActorComponent::StaticClass_ = nullptr;
const NClass *nilou::UActorComponent::GetClass() const 
{ 
    return nilou::UActorComponent::StaticClass(); 
}
const NClass *nilou::UActorComponent::StaticClass()
{
    return nilou::UActorComponent::StaticClass_.get();
}

template<>
struct TClassRegistry<nilou::UActorComponent>
{
    TClassRegistry(const std::string& InName)
    {
        nilou::UActorComponent::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<nilou::UActorComponent>();
		Mngr.AddBases<nilou::UActorComponent, NObject>();
;
        nilou::UActorComponent::StaticClass_->Type = Type_of<nilou::UActorComponent>;
        nilou::UActorComponent::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<nilou::UActorComponent>);
    }

    static TClassRegistry<nilou::UActorComponent> Dummy;
};
TClassRegistry<nilou::UActorComponent> Dummy = TClassRegistry<nilou::UActorComponent>("nilou::UActorComponent");



void nilou::UActorComponent::Serialize(FArchive& Ar)
{
    NObject::Serialize(Ar);
    if (this->bIsSerializing)
        return;
    this->bIsSerializing = true;
    nlohmann::json& Node = Ar.Node;
    Node["ClassName"] = "nilou::UActorComponent";
    nlohmann::json &content = Node["Content"];

    this->bIsSerializing = false;
}

void nilou::UActorComponent::Deserialize(FArchive& Ar)
{
    nlohmann::json& Node = Ar.Node;
    nlohmann::json &content = Node["Content"];

    NObject::Deserialize(Ar);
}
