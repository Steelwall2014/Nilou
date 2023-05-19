#include "D:/Nilou/src/Runtime/Cesium3DTiles/Cesium3DTileComponent.h"
#include <UDRefl/UDRefl.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> nilou::UCesium3DTileComponent::StaticClass_ = nullptr;
const NClass *nilou::UCesium3DTileComponent::GetClass() const 
{ 
    return nilou::UCesium3DTileComponent::StaticClass(); 
}
const NClass *nilou::UCesium3DTileComponent::StaticClass()
{
    return nilou::UCesium3DTileComponent::StaticClass_.get();
}

template<>
struct TClassRegistry<nilou::UCesium3DTileComponent>
{
    TClassRegistry(const std::string& InName)
    {
        nilou::UCesium3DTileComponent::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<nilou::UCesium3DTileComponent>();
		Mngr.AddBases<nilou::UCesium3DTileComponent, nilou::UPrimitiveComponent>();
;
        nilou::UCesium3DTileComponent::StaticClass_->Type = Type_of<nilou::UCesium3DTileComponent>;
        nilou::UCesium3DTileComponent::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<nilou::UCesium3DTileComponent>);
    }

    static TClassRegistry<nilou::UCesium3DTileComponent> Dummy;
};
TClassRegistry<nilou::UCesium3DTileComponent> Dummy = TClassRegistry<nilou::UCesium3DTileComponent>("nilou::UCesium3DTileComponent");



void nilou::UCesium3DTileComponent::Serialize(FArchive& Ar)
{
    nilou::UPrimitiveComponent::Serialize(Ar);
    if (this->bIsSerializing)
        return;
    this->bIsSerializing = true;
    nlohmann::json& Node = Ar.Node;
    Node["ClassName"] = "nilou::UCesium3DTileComponent";
    nlohmann::json &content = Node["Content"];

    this->bIsSerializing = false;
}

void nilou::UCesium3DTileComponent::Deserialize(FArchive& Ar)
{
    nlohmann::json& Node = Ar.Node;
    nlohmann::json &content = Node["Content"];

    nilou::UPrimitiveComponent::Deserialize(Ar);
}
