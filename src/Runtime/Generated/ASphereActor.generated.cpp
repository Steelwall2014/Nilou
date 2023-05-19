#include "D:/Nilou/src/Runtime/Framework/Common/Actor/SphereActor.h"
#include <UDRefl/UDRefl.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> nilou::ASphereActor::StaticClass_ = nullptr;
const NClass *nilou::ASphereActor::GetClass() const 
{ 
    return nilou::ASphereActor::StaticClass(); 
}
const NClass *nilou::ASphereActor::StaticClass()
{
    return nilou::ASphereActor::StaticClass_.get();
}

template<>
struct TClassRegistry<nilou::ASphereActor>
{
    TClassRegistry(const std::string& InName)
    {
        nilou::ASphereActor::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<nilou::ASphereActor>();
		Mngr.AddBases<nilou::ASphereActor, nilou::AActor>();
;
        nilou::ASphereActor::StaticClass_->Type = Type_of<nilou::ASphereActor>;
        nilou::ASphereActor::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<nilou::ASphereActor>);
    }

    static TClassRegistry<nilou::ASphereActor> Dummy;
};
TClassRegistry<nilou::ASphereActor> Dummy = TClassRegistry<nilou::ASphereActor>("nilou::ASphereActor");



void nilou::ASphereActor::Serialize(FArchive& Ar)
{
    nilou::AActor::Serialize(Ar);
    if (this->bIsSerializing)
        return;
    this->bIsSerializing = true;
    nlohmann::json& Node = Ar.Node;
    Node["ClassName"] = "nilou::ASphereActor";
    nlohmann::json &content = Node["Content"];

    this->bIsSerializing = false;
}

void nilou::ASphereActor::Deserialize(FArchive& Ar)
{
    nlohmann::json& Node = Ar.Node;
    nlohmann::json &content = Node["Content"];

    nilou::AActor::Deserialize(Ar);
}
