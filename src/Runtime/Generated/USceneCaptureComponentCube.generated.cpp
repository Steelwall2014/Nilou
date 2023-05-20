#include "D:/Nilou/src/Runtime/Framework/Common/Components/SceneCaptureComponent.h"
#include <UDRefl/UDRefl.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> nilou::USceneCaptureComponentCube::StaticClass_ = nullptr;
const NClass *nilou::USceneCaptureComponentCube::GetClass() const 
{ 
    return nilou::USceneCaptureComponentCube::StaticClass(); 
}
const NClass *nilou::USceneCaptureComponentCube::StaticClass()
{
    return nilou::USceneCaptureComponentCube::StaticClass_.get();
}

template<>
struct TClassRegistry<nilou::USceneCaptureComponentCube>
{
    TClassRegistry(const std::string& InName)
    {
        nilou::USceneCaptureComponentCube::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<nilou::USceneCaptureComponentCube>();
		Mngr.AddBases<nilou::USceneCaptureComponentCube, nilou::USceneCaptureComponent>();
;
        nilou::USceneCaptureComponentCube::StaticClass_->Type = Type_of<nilou::USceneCaptureComponentCube>;
        nilou::USceneCaptureComponentCube::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<nilou::USceneCaptureComponentCube>);
    }

    static TClassRegistry<nilou::USceneCaptureComponentCube> Dummy;
};
TClassRegistry<nilou::USceneCaptureComponentCube> Dummy = TClassRegistry<nilou::USceneCaptureComponentCube>("nilou::USceneCaptureComponentCube");



void nilou::USceneCaptureComponentCube::Serialize(FArchive& Ar)
{
    nilou::USceneCaptureComponent::Serialize(Ar);
    if (this->bIsSerializing)
        return;
    this->bIsSerializing = true;
    nlohmann::json& Node = Ar.Node;
    Node["ClassName"] = "nilou::USceneCaptureComponentCube";
    nlohmann::json &content = Node["Content"];

    this->bIsSerializing = false;
}

void nilou::USceneCaptureComponentCube::Deserialize(FArchive& Ar)
{
    nlohmann::json& Node = Ar.Node;
    nlohmann::json &content = Node["Content"];

    nilou::USceneCaptureComponent::Deserialize(Ar);
}
