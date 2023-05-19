#include "D:/Nilou/src/Runtime/Framework/Common/Actor/LineBatchActor.h"
#include <UDRefl/UDRefl.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> nilou::ALineBatchActor::StaticClass_ = nullptr;
const NClass *nilou::ALineBatchActor::GetClass() const 
{ 
    return nilou::ALineBatchActor::StaticClass(); 
}
const NClass *nilou::ALineBatchActor::StaticClass()
{
    return nilou::ALineBatchActor::StaticClass_.get();
}

template<>
struct TClassRegistry<nilou::ALineBatchActor>
{
    TClassRegistry(const std::string& InName)
    {
        nilou::ALineBatchActor::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<nilou::ALineBatchActor>();
		Mngr.AddBases<nilou::ALineBatchActor, nilou::AActor>();
;
        nilou::ALineBatchActor::StaticClass_->Type = Type_of<nilou::ALineBatchActor>;
        nilou::ALineBatchActor::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<nilou::ALineBatchActor>);
    }

    static TClassRegistry<nilou::ALineBatchActor> Dummy;
};
TClassRegistry<nilou::ALineBatchActor> Dummy = TClassRegistry<nilou::ALineBatchActor>("nilou::ALineBatchActor");



void nilou::ALineBatchActor::Serialize(FArchive& Ar)
{
    nilou::AActor::Serialize(Ar);
    if (this->bIsSerializing)
        return;
    this->bIsSerializing = true;
    nlohmann::json& Node = Ar.Node;
    Node["ClassName"] = "nilou::ALineBatchActor";
    nlohmann::json &content = Node["Content"];

    this->bIsSerializing = false;
}

void nilou::ALineBatchActor::Deserialize(FArchive& Ar)
{
    nlohmann::json& Node = Ar.Node;
    nlohmann::json &content = Node["Content"];

    nilou::AActor::Deserialize(Ar);
}
