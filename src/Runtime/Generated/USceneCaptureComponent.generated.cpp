#include "D:/Nilou/src/Runtime/Framework/Common/Components/ReflectionProbeComponent.h"
#include <UDRefl/UDRefl.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> nilou::USceneCaptureComponent::StaticClass_ = nullptr;
const NClass *nilou::USceneCaptureComponent::GetClass() const 
{ 
    return nilou::USceneCaptureComponent::StaticClass(); 
}
const NClass *nilou::USceneCaptureComponent::StaticClass()
{
    return nilou::USceneCaptureComponent::StaticClass_.get();
}

template<>
struct TClassRegistry<nilou::USceneCaptureComponent>
{
    TClassRegistry(const std::string& InName)
    {
        nilou::USceneCaptureComponent::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<nilou::USceneCaptureComponent>();
		Mngr.AddBases<nilou::USceneCaptureComponent, nilou::USceneComponent>();
;
        nilou::USceneCaptureComponent::StaticClass_->Type = Type_of<nilou::USceneCaptureComponent>;
        nilou::USceneCaptureComponent::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<nilou::USceneCaptureComponent>);
    }

    static TClassRegistry<nilou::USceneCaptureComponent> Dummy;
};
TClassRegistry<nilou::USceneCaptureComponent> Dummy = TClassRegistry<nilou::USceneCaptureComponent>("nilou::USceneCaptureComponent");



void nilou::USceneCaptureComponent::Serialize(FArchive& Ar)
{
    nilou::USceneComponent::Serialize(Ar);
    if (this->bIsSerializing)
        return;
    this->bIsSerializing = true;
    nlohmann::json& Node = Ar.Node;
    Node["ClassName"] = "nilou::USceneCaptureComponent";
    nlohmann::json &content = Node["Content"];

    this->bIsSerializing = false;
}

void nilou::USceneCaptureComponent::Deserialize(FArchive& Ar)
{
    nlohmann::json& Node = Ar.Node;
    nlohmann::json &content = Node["Content"];

    nilou::USceneComponent::Deserialize(Ar);
}
