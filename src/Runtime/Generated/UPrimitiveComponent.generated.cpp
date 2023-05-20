#include "D:/Nilou/src/Runtime/Framework/Common/Components/SceneCaptureComponent.h"
#include <UDRefl/UDRefl.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> nilou::UPrimitiveComponent::StaticClass_ = nullptr;
const NClass *nilou::UPrimitiveComponent::GetClass() const 
{ 
    return nilou::UPrimitiveComponent::StaticClass(); 
}
const NClass *nilou::UPrimitiveComponent::StaticClass()
{
    return nilou::UPrimitiveComponent::StaticClass_.get();
}

template<>
struct TClassRegistry<nilou::UPrimitiveComponent>
{
    TClassRegistry(const std::string& InName)
    {
        nilou::UPrimitiveComponent::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<nilou::UPrimitiveComponent>();
		Mngr.AddBases<nilou::UPrimitiveComponent, nilou::USceneComponent>();
;
        nilou::UPrimitiveComponent::StaticClass_->Type = Type_of<nilou::UPrimitiveComponent>;
        nilou::UPrimitiveComponent::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<nilou::UPrimitiveComponent>);
    }

    static TClassRegistry<nilou::UPrimitiveComponent> Dummy;
};
TClassRegistry<nilou::UPrimitiveComponent> Dummy = TClassRegistry<nilou::UPrimitiveComponent>("nilou::UPrimitiveComponent");



void nilou::UPrimitiveComponent::Serialize(FArchive& Ar)
{
    nilou::USceneComponent::Serialize(Ar);
    if (this->bIsSerializing)
        return;
    this->bIsSerializing = true;
    nlohmann::json& Node = Ar.Node;
    Node["ClassName"] = "nilou::UPrimitiveComponent";
    nlohmann::json &content = Node["Content"];

    this->bIsSerializing = false;
}

void nilou::UPrimitiveComponent::Deserialize(FArchive& Ar)
{
    nlohmann::json& Node = Ar.Node;
    nlohmann::json &content = Node["Content"];

    nilou::USceneComponent::Deserialize(Ar);
}
