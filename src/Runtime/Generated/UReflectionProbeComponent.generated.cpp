#include "D:/Nilou/src/Runtime/Framework/Common/Components/FourierTransformOcean.h"
#include <UDRefl/UDRefl.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> nilou::UReflectionProbeComponent::StaticClass_ = nullptr;
const NClass *nilou::UReflectionProbeComponent::GetClass() const 
{ 
    return nilou::UReflectionProbeComponent::StaticClass(); 
}
const NClass *nilou::UReflectionProbeComponent::StaticClass()
{
    return nilou::UReflectionProbeComponent::StaticClass_.get();
}

template<>
struct TClassRegistry<nilou::UReflectionProbeComponent>
{
    TClassRegistry(const std::string& InName)
    {
        nilou::UReflectionProbeComponent::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<nilou::UReflectionProbeComponent>();
		Mngr.AddBases<nilou::UReflectionProbeComponent, nilou::USceneCaptureComponentCube>();
;
        nilou::UReflectionProbeComponent::StaticClass_->Type = Type_of<nilou::UReflectionProbeComponent>;
        nilou::UReflectionProbeComponent::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<nilou::UReflectionProbeComponent>);
    }

    static TClassRegistry<nilou::UReflectionProbeComponent> Dummy;
};
TClassRegistry<nilou::UReflectionProbeComponent> Dummy = TClassRegistry<nilou::UReflectionProbeComponent>("nilou::UReflectionProbeComponent");



void nilou::UReflectionProbeComponent::Serialize(FArchive& Ar)
{
    nilou::USceneCaptureComponentCube::Serialize(Ar);
    if (this->bIsSerializing)
        return;
    this->bIsSerializing = true;
    nlohmann::json& Node = Ar.Node;
    Node["ClassName"] = "nilou::UReflectionProbeComponent";
    nlohmann::json &content = Node["Content"];

    this->bIsSerializing = false;
}

void nilou::UReflectionProbeComponent::Deserialize(FArchive& Ar)
{
    nlohmann::json& Node = Ar.Node;
    nlohmann::json &content = Node["Content"];

    nilou::USceneCaptureComponentCube::Deserialize(Ar);
}
