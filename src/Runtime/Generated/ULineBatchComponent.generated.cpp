#include "D:/Nilou/src/Runtime/Framework/Common/Components/LineBatchComponent.h"
#include <UDRefl/UDRefl.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> nilou::ULineBatchComponent::StaticClass_ = nullptr;
const NClass *nilou::ULineBatchComponent::GetClass() const 
{ 
    return nilou::ULineBatchComponent::StaticClass(); 
}
const NClass *nilou::ULineBatchComponent::StaticClass()
{
    return nilou::ULineBatchComponent::StaticClass_.get();
}

template<>
struct TClassRegistry<nilou::ULineBatchComponent>
{
    TClassRegistry(const std::string& InName)
    {
        nilou::ULineBatchComponent::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<nilou::ULineBatchComponent>();
		Mngr.AddBases<nilou::ULineBatchComponent, nilou::UPrimitiveComponent>();
;
        nilou::ULineBatchComponent::StaticClass_->Type = Type_of<nilou::ULineBatchComponent>;
        nilou::ULineBatchComponent::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<nilou::ULineBatchComponent>);
    }

    static TClassRegistry<nilou::ULineBatchComponent> Dummy;
};
TClassRegistry<nilou::ULineBatchComponent> Dummy = TClassRegistry<nilou::ULineBatchComponent>("nilou::ULineBatchComponent");



void nilou::ULineBatchComponent::Serialize(FArchive& Ar)
{
    nilou::UPrimitiveComponent::Serialize(Ar);
    if (this->bIsSerializing)
        return;
    this->bIsSerializing = true;
    nlohmann::json& Node = Ar.Node;
    Node["ClassName"] = "nilou::ULineBatchComponent";
    nlohmann::json &content = Node["Content"];

    this->bIsSerializing = false;
}

void nilou::ULineBatchComponent::Deserialize(FArchive& Ar)
{
    nlohmann::json& Node = Ar.Node;
    nlohmann::json &content = Node["Content"];

    nilou::UPrimitiveComponent::Deserialize(Ar);
}
