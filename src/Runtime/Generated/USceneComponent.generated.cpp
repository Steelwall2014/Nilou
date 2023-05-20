#include "D:/Nilou/src/Runtime/Framework/Common/Actor/SkyAtmosphereActor.h"
#include <UDRefl/UDRefl.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> nilou::USceneComponent::StaticClass_ = nullptr;
const NClass *nilou::USceneComponent::GetClass() const 
{ 
    return nilou::USceneComponent::StaticClass(); 
}
const NClass *nilou::USceneComponent::StaticClass()
{
    return nilou::USceneComponent::StaticClass_.get();
}

template<>
struct TClassRegistry<nilou::USceneComponent>
{
    TClassRegistry(const std::string& InName)
    {
        nilou::USceneComponent::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<nilou::USceneComponent>();
		Mngr.AddBases<nilou::USceneComponent, nilou::UActorComponent>();
;
        nilou::USceneComponent::StaticClass_->Type = Type_of<nilou::USceneComponent>;
        nilou::USceneComponent::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<nilou::USceneComponent>);
    }

    static TClassRegistry<nilou::USceneComponent> Dummy;
};
TClassRegistry<nilou::USceneComponent> Dummy = TClassRegistry<nilou::USceneComponent>("nilou::USceneComponent");



void nilou::USceneComponent::Serialize(FArchive& Ar)
{
    nilou::UActorComponent::Serialize(Ar);
    if (this->bIsSerializing)
        return;
    this->bIsSerializing = true;
    nlohmann::json& Node = Ar.Node;
    Node["ClassName"] = "nilou::USceneComponent";
    nlohmann::json &content = Node["Content"];

    this->bIsSerializing = false;
}

void nilou::USceneComponent::Deserialize(FArchive& Ar)
{
    nlohmann::json& Node = Ar.Node;
    nlohmann::json &content = Node["Content"];

    nilou::UActorComponent::Deserialize(Ar);
}
