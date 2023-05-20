#include "D:/Nilou/src/Runtime/Framework/Common/Components/FourierTransformOcean.h"
#include <UDRefl/UDRefl.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> nilou::ULightComponent::StaticClass_ = nullptr;
const NClass *nilou::ULightComponent::GetClass() const 
{ 
    return nilou::ULightComponent::StaticClass(); 
}
const NClass *nilou::ULightComponent::StaticClass()
{
    return nilou::ULightComponent::StaticClass_.get();
}

template<>
struct TClassRegistry<nilou::ULightComponent>
{
    TClassRegistry(const std::string& InName)
    {
        nilou::ULightComponent::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<nilou::ULightComponent>();
		Mngr.AddBases<nilou::ULightComponent, nilou::USceneComponent>();
;
        nilou::ULightComponent::StaticClass_->Type = Type_of<nilou::ULightComponent>;
        nilou::ULightComponent::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<nilou::ULightComponent>);
    }

    static TClassRegistry<nilou::ULightComponent> Dummy;
};
TClassRegistry<nilou::ULightComponent> Dummy = TClassRegistry<nilou::ULightComponent>("nilou::ULightComponent");



void nilou::ULightComponent::Serialize(FArchive& Ar)
{
    nilou::USceneComponent::Serialize(Ar);
    if (this->bIsSerializing)
        return;
    this->bIsSerializing = true;
    nlohmann::json& Node = Ar.Node;
    Node["ClassName"] = "nilou::ULightComponent";
    nlohmann::json &content = Node["Content"];

    this->bIsSerializing = false;
}

void nilou::ULightComponent::Deserialize(FArchive& Ar)
{
    nlohmann::json& Node = Ar.Node;
    nlohmann::json &content = Node["Content"];

    nilou::USceneComponent::Deserialize(Ar);
}
