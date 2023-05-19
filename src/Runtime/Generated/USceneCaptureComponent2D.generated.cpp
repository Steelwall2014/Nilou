#include "D:/Nilou/src/Runtime/Framework/Common/Components/ReflectionProbeComponent.h"
#include <UDRefl/UDRefl.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> nilou::USceneCaptureComponent2D::StaticClass_ = nullptr;
const NClass *nilou::USceneCaptureComponent2D::GetClass() const 
{ 
    return nilou::USceneCaptureComponent2D::StaticClass(); 
}
const NClass *nilou::USceneCaptureComponent2D::StaticClass()
{
    return nilou::USceneCaptureComponent2D::StaticClass_.get();
}

template<>
struct TClassRegistry<nilou::USceneCaptureComponent2D>
{
    TClassRegistry(const std::string& InName)
    {
        nilou::USceneCaptureComponent2D::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<nilou::USceneCaptureComponent2D>();
		Mngr.AddBases<nilou::USceneCaptureComponent2D, nilou::USceneCaptureComponent>();
;
        nilou::USceneCaptureComponent2D::StaticClass_->Type = Type_of<nilou::USceneCaptureComponent2D>;
        nilou::USceneCaptureComponent2D::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<nilou::USceneCaptureComponent2D>);
    }

    static TClassRegistry<nilou::USceneCaptureComponent2D> Dummy;
};
TClassRegistry<nilou::USceneCaptureComponent2D> Dummy = TClassRegistry<nilou::USceneCaptureComponent2D>("nilou::USceneCaptureComponent2D");



void nilou::USceneCaptureComponent2D::Serialize(FArchive& Ar)
{
    nilou::USceneCaptureComponent::Serialize(Ar);
    if (this->bIsSerializing)
        return;
    this->bIsSerializing = true;
    nlohmann::json& Node = Ar.Node;
    Node["ClassName"] = "nilou::USceneCaptureComponent2D";
    nlohmann::json &content = Node["Content"];

    this->bIsSerializing = false;
}

void nilou::USceneCaptureComponent2D::Deserialize(FArchive& Ar)
{
    nlohmann::json& Node = Ar.Node;
    nlohmann::json &content = Node["Content"];

    nilou::USceneCaptureComponent::Deserialize(Ar);
}
