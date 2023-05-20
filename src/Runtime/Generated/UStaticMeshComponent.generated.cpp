#include "D:/Nilou/src/Runtime/Framework/Common/Actor/StaticMeshActor.h"
#include <UDRefl/UDRefl.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> nilou::UStaticMeshComponent::StaticClass_ = nullptr;
const NClass *nilou::UStaticMeshComponent::GetClass() const 
{ 
    return nilou::UStaticMeshComponent::StaticClass(); 
}
const NClass *nilou::UStaticMeshComponent::StaticClass()
{
    return nilou::UStaticMeshComponent::StaticClass_.get();
}

template<>
struct TClassRegistry<nilou::UStaticMeshComponent>
{
    TClassRegistry(const std::string& InName)
    {
        nilou::UStaticMeshComponent::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<nilou::UStaticMeshComponent>();
		Mngr.AddBases<nilou::UStaticMeshComponent, nilou::UPrimitiveComponent>();
;
        nilou::UStaticMeshComponent::StaticClass_->Type = Type_of<nilou::UStaticMeshComponent>;
        nilou::UStaticMeshComponent::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<nilou::UStaticMeshComponent>);
    }

    static TClassRegistry<nilou::UStaticMeshComponent> Dummy;
};
TClassRegistry<nilou::UStaticMeshComponent> Dummy = TClassRegistry<nilou::UStaticMeshComponent>("nilou::UStaticMeshComponent");



void nilou::UStaticMeshComponent::Serialize(FArchive& Ar)
{
    nilou::UPrimitiveComponent::Serialize(Ar);
    if (this->bIsSerializing)
        return;
    this->bIsSerializing = true;
    nlohmann::json& Node = Ar.Node;
    Node["ClassName"] = "nilou::UStaticMeshComponent";
    nlohmann::json &content = Node["Content"];

    this->bIsSerializing = false;
}

void nilou::UStaticMeshComponent::Deserialize(FArchive& Ar)
{
    nlohmann::json& Node = Ar.Node;
    nlohmann::json &content = Node["Content"];

    nilou::UPrimitiveComponent::Deserialize(Ar);
}
