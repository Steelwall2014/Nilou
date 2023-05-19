#include "D:/Nilou/src/Runtime/Framework/Common/Actor/VirtualHeightfieldMeshActor.h"
#include <UDRefl/UDRefl.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> nilou::AVirtualHeightfieldMeshActor::StaticClass_ = nullptr;
const NClass *nilou::AVirtualHeightfieldMeshActor::GetClass() const 
{ 
    return nilou::AVirtualHeightfieldMeshActor::StaticClass(); 
}
const NClass *nilou::AVirtualHeightfieldMeshActor::StaticClass()
{
    return nilou::AVirtualHeightfieldMeshActor::StaticClass_.get();
}

template<>
struct TClassRegistry<nilou::AVirtualHeightfieldMeshActor>
{
    TClassRegistry(const std::string& InName)
    {
        nilou::AVirtualHeightfieldMeshActor::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<nilou::AVirtualHeightfieldMeshActor>();
		Mngr.AddBases<nilou::AVirtualHeightfieldMeshActor, nilou::AActor>();
;
        nilou::AVirtualHeightfieldMeshActor::StaticClass_->Type = Type_of<nilou::AVirtualHeightfieldMeshActor>;
        nilou::AVirtualHeightfieldMeshActor::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<nilou::AVirtualHeightfieldMeshActor>);
    }

    static TClassRegistry<nilou::AVirtualHeightfieldMeshActor> Dummy;
};
TClassRegistry<nilou::AVirtualHeightfieldMeshActor> Dummy = TClassRegistry<nilou::AVirtualHeightfieldMeshActor>("nilou::AVirtualHeightfieldMeshActor");



void nilou::AVirtualHeightfieldMeshActor::Serialize(FArchive& Ar)
{
    nilou::AActor::Serialize(Ar);
    if (this->bIsSerializing)
        return;
    this->bIsSerializing = true;
    nlohmann::json& Node = Ar.Node;
    Node["ClassName"] = "nilou::AVirtualHeightfieldMeshActor";
    nlohmann::json &content = Node["Content"];

    this->bIsSerializing = false;
}

void nilou::AVirtualHeightfieldMeshActor::Deserialize(FArchive& Ar)
{
    nlohmann::json& Node = Ar.Node;
    nlohmann::json &content = Node["Content"];

    nilou::AActor::Deserialize(Ar);
}
