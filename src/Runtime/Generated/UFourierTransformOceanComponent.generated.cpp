#include "D:/Nilou/src/Runtime/Framework/Common/Components/FourierTransformOcean.h"
#include <UDRefl/UDRefl.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> nilou::UFourierTransformOceanComponent::StaticClass_ = nullptr;
const NClass *nilou::UFourierTransformOceanComponent::GetClass() const 
{ 
    return nilou::UFourierTransformOceanComponent::StaticClass(); 
}
const NClass *nilou::UFourierTransformOceanComponent::StaticClass()
{
    return nilou::UFourierTransformOceanComponent::StaticClass_.get();
}

template<>
struct TClassRegistry<nilou::UFourierTransformOceanComponent>
{
    TClassRegistry(const std::string& InName)
    {
        nilou::UFourierTransformOceanComponent::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<nilou::UFourierTransformOceanComponent>();
		Mngr.AddBases<nilou::UFourierTransformOceanComponent, nilou::UPrimitiveComponent>();
;
        nilou::UFourierTransformOceanComponent::StaticClass_->Type = Type_of<nilou::UFourierTransformOceanComponent>;
        nilou::UFourierTransformOceanComponent::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<nilou::UFourierTransformOceanComponent>);
    }

    static TClassRegistry<nilou::UFourierTransformOceanComponent> Dummy;
};
TClassRegistry<nilou::UFourierTransformOceanComponent> Dummy = TClassRegistry<nilou::UFourierTransformOceanComponent>("nilou::UFourierTransformOceanComponent");



void nilou::UFourierTransformOceanComponent::Serialize(FArchive& Ar)
{
    nilou::UPrimitiveComponent::Serialize(Ar);
    if (this->bIsSerializing)
        return;
    this->bIsSerializing = true;
    nlohmann::json& Node = Ar.Node;
    Node["ClassName"] = "nilou::UFourierTransformOceanComponent";
    nlohmann::json &content = Node["Content"];

    this->bIsSerializing = false;
}

void nilou::UFourierTransformOceanComponent::Deserialize(FArchive& Ar)
{
    nlohmann::json& Node = Ar.Node;
    nlohmann::json &content = Node["Content"];

    nilou::UPrimitiveComponent::Deserialize(Ar);
}
