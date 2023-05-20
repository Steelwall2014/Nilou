#include "D:/Nilou/src/Runtime/Framework/Common/Components/FourierTransformOcean.h"
#include <UDRefl/UDRefl.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> nilou::UCameraComponent::StaticClass_ = nullptr;
const NClass *nilou::UCameraComponent::GetClass() const 
{ 
    return nilou::UCameraComponent::StaticClass(); 
}
const NClass *nilou::UCameraComponent::StaticClass()
{
    return nilou::UCameraComponent::StaticClass_.get();
}

template<>
struct TClassRegistry<nilou::UCameraComponent>
{
    TClassRegistry(const std::string& InName)
    {
        nilou::UCameraComponent::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<nilou::UCameraComponent>();
		Mngr.AddBases<nilou::UCameraComponent, nilou::USceneComponent>();
;
        nilou::UCameraComponent::StaticClass_->Type = Type_of<nilou::UCameraComponent>;
        nilou::UCameraComponent::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<nilou::UCameraComponent>);
    }

    static TClassRegistry<nilou::UCameraComponent> Dummy;
};
TClassRegistry<nilou::UCameraComponent> Dummy = TClassRegistry<nilou::UCameraComponent>("nilou::UCameraComponent");



void nilou::UCameraComponent::Serialize(FArchive& Ar)
{
    nilou::USceneComponent::Serialize(Ar);
    if (this->bIsSerializing)
        return;
    this->bIsSerializing = true;
    nlohmann::json& Node = Ar.Node;
    Node["ClassName"] = "nilou::UCameraComponent";
    nlohmann::json &content = Node["Content"];

    this->bIsSerializing = false;
}

void nilou::UCameraComponent::Deserialize(FArchive& Ar)
{
    nlohmann::json& Node = Ar.Node;
    nlohmann::json &content = Node["Content"];

    nilou::USceneComponent::Deserialize(Ar);
}
