#include "D:/Nilou/src/Runtime/Framework/Common/Components/ArrowComponent.h"
#include <UDRefl/UDRefl.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> nilou::UArrowComponent::StaticClass_ = nullptr;
const NClass *nilou::UArrowComponent::GetClass() const 
{ 
    return nilou::UArrowComponent::StaticClass(); 
}
const NClass *nilou::UArrowComponent::StaticClass()
{
    return nilou::UArrowComponent::StaticClass_.get();
}

template<>
struct TClassRegistry<nilou::UArrowComponent>
{
    TClassRegistry(const std::string& InName)
    {
        nilou::UArrowComponent::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<nilou::UArrowComponent>();
		Mngr.AddBases<nilou::UArrowComponent, nilou::UPrimitiveComponent>();
;
        nilou::UArrowComponent::StaticClass_->Type = Type_of<nilou::UArrowComponent>;
        nilou::UArrowComponent::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<nilou::UArrowComponent>);
    }

    static TClassRegistry<nilou::UArrowComponent> Dummy;
};
TClassRegistry<nilou::UArrowComponent> Dummy = TClassRegistry<nilou::UArrowComponent>("nilou::UArrowComponent");



void nilou::UArrowComponent::Serialize(FArchive& Ar)
{
    nilou::UPrimitiveComponent::Serialize(Ar);
    if (this->bIsSerializing)
        return;
    this->bIsSerializing = true;
    nlohmann::json& Node = Ar.Node;
    Node["ClassName"] = "nilou::UArrowComponent";
    nlohmann::json &content = Node["Content"];

    this->bIsSerializing = false;
}

void nilou::UArrowComponent::Deserialize(FArchive& Ar)
{
    nlohmann::json& Node = Ar.Node;
    nlohmann::json &content = Node["Content"];

    nilou::UPrimitiveComponent::Deserialize(Ar);
}
