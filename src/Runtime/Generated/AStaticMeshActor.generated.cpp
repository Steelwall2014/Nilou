#include "D:/Nilou/src/Runtime/Framework/Common/Actor/StaticMeshActor.h"
#include <UDRefl/UDRefl.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> nilou::AStaticMeshActor::StaticClass_ = nullptr;
const NClass *nilou::AStaticMeshActor::GetClass() const 
{ 
    return nilou::AStaticMeshActor::StaticClass(); 
}
const NClass *nilou::AStaticMeshActor::StaticClass()
{
    return nilou::AStaticMeshActor::StaticClass_.get();
}

template<>
struct TClassRegistry<nilou::AStaticMeshActor>
{
    TClassRegistry(const std::string& InName)
    {
        nilou::AStaticMeshActor::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<nilou::AStaticMeshActor>();
		Mngr.AddBases<nilou::AStaticMeshActor, nilou::AActor>();
;
        nilou::AStaticMeshActor::StaticClass_->Type = Type_of<nilou::AStaticMeshActor>;
        nilou::AStaticMeshActor::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<nilou::AStaticMeshActor>);
    }

    static TClassRegistry<nilou::AStaticMeshActor> Dummy;
};
TClassRegistry<nilou::AStaticMeshActor> Dummy = TClassRegistry<nilou::AStaticMeshActor>("nilou::AStaticMeshActor");



void nilou::AStaticMeshActor::Serialize(FArchive& Ar)
{
    nilou::AActor::Serialize(Ar);
    if (this->bIsSerializing)
        return;
    this->bIsSerializing = true;
    nlohmann::json& Node = Ar.Node;
    Node["ClassName"] = "nilou::AStaticMeshActor";
    nlohmann::json &content = Node["Content"];

    this->bIsSerializing = false;
}

void nilou::AStaticMeshActor::Deserialize(FArchive& Ar)
{
    nlohmann::json& Node = Ar.Node;
    nlohmann::json &content = Node["Content"];

    nilou::AActor::Deserialize(Ar);
}
