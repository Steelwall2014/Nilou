#include "D:/Nilou/src/Runtime/Framework/Common/Actor/CameraActor.h"
#include <UDRefl/UDRefl.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> nilou::ACameraActor::StaticClass_ = nullptr;
const NClass *nilou::ACameraActor::GetClass() const 
{ 
    return nilou::ACameraActor::StaticClass(); 
}
const NClass *nilou::ACameraActor::StaticClass()
{
    return nilou::ACameraActor::StaticClass_.get();
}

template<>
struct TClassRegistry<nilou::ACameraActor>
{
    TClassRegistry(const std::string& InName)
    {
        nilou::ACameraActor::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<nilou::ACameraActor>();
		Mngr.AddBases<nilou::ACameraActor, nilou::AActor>();
;
        nilou::ACameraActor::StaticClass_->Type = Type_of<nilou::ACameraActor>;
        nilou::ACameraActor::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<nilou::ACameraActor>);
    }

    static TClassRegistry<nilou::ACameraActor> Dummy;
};
TClassRegistry<nilou::ACameraActor> Dummy = TClassRegistry<nilou::ACameraActor>("nilou::ACameraActor");



void nilou::ACameraActor::Serialize(FArchive& Ar)
{
    nilou::AActor::Serialize(Ar);
    if (this->bIsSerializing)
        return;
    this->bIsSerializing = true;
    nlohmann::json& Node = Ar.Node;
    Node["ClassName"] = "nilou::ACameraActor";
    nlohmann::json &content = Node["Content"];

    this->bIsSerializing = false;
}

void nilou::ACameraActor::Deserialize(FArchive& Ar)
{
    nlohmann::json& Node = Ar.Node;
    nlohmann::json &content = Node["Content"];

    nilou::AActor::Deserialize(Ar);
}
