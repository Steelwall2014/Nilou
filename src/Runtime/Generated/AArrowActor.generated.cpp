#include "D:/Nilou/src/Runtime/Framework/Common/Actor/ArrowActor.h"
#include <UDRefl/UDRefl.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> nilou::AArrowActor::StaticClass_ = nullptr;
const NClass *nilou::AArrowActor::GetClass() const 
{ 
    return nilou::AArrowActor::StaticClass(); 
}
const NClass *nilou::AArrowActor::StaticClass()
{
    return nilou::AArrowActor::StaticClass_.get();
}

template<>
struct TClassRegistry<nilou::AArrowActor>
{
    TClassRegistry(const std::string& InName)
    {
        nilou::AArrowActor::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<nilou::AArrowActor>();
		Mngr.AddBases<nilou::AArrowActor, nilou::AActor>();
;
        nilou::AArrowActor::StaticClass_->Type = Type_of<nilou::AArrowActor>;
        nilou::AArrowActor::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<nilou::AArrowActor>);
    }

    static TClassRegistry<nilou::AArrowActor> Dummy;
};
TClassRegistry<nilou::AArrowActor> Dummy = TClassRegistry<nilou::AArrowActor>("nilou::AArrowActor");



void nilou::AArrowActor::Serialize(FArchive& Ar)
{
    nilou::AActor::Serialize(Ar);
    if (this->bIsSerializing)
        return;
    this->bIsSerializing = true;
    nlohmann::json& Node = Ar.Node;
    Node["ClassName"] = "nilou::AArrowActor";
    nlohmann::json &content = Node["Content"];

    this->bIsSerializing = false;
}

void nilou::AArrowActor::Deserialize(FArchive& Ar)
{
    nlohmann::json& Node = Ar.Node;
    nlohmann::json &content = Node["Content"];

    nilou::AActor::Deserialize(Ar);
}
