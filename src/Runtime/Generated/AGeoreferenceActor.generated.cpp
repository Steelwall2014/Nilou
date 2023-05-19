#include "D:/Nilou/src/Runtime/Cesium3DTiles/Cesium3DTileset.h"
#include <UDRefl/UDRefl.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> nilou::AGeoreferenceActor::StaticClass_ = nullptr;
const NClass *nilou::AGeoreferenceActor::GetClass() const 
{ 
    return nilou::AGeoreferenceActor::StaticClass(); 
}
const NClass *nilou::AGeoreferenceActor::StaticClass()
{
    return nilou::AGeoreferenceActor::StaticClass_.get();
}

template<>
struct TClassRegistry<nilou::AGeoreferenceActor>
{
    TClassRegistry(const std::string& InName)
    {
        nilou::AGeoreferenceActor::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<nilou::AGeoreferenceActor>();
		Mngr.AddBases<nilou::AGeoreferenceActor, nilou::AActor>();
;
        nilou::AGeoreferenceActor::StaticClass_->Type = Type_of<nilou::AGeoreferenceActor>;
        nilou::AGeoreferenceActor::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<nilou::AGeoreferenceActor>);
    }

    static TClassRegistry<nilou::AGeoreferenceActor> Dummy;
};
TClassRegistry<nilou::AGeoreferenceActor> Dummy = TClassRegistry<nilou::AGeoreferenceActor>("nilou::AGeoreferenceActor");



void nilou::AGeoreferenceActor::Serialize(FArchive& Ar)
{
    nilou::AActor::Serialize(Ar);
    if (this->bIsSerializing)
        return;
    this->bIsSerializing = true;
    nlohmann::json& Node = Ar.Node;
    Node["ClassName"] = "nilou::AGeoreferenceActor";
    nlohmann::json &content = Node["Content"];

    this->bIsSerializing = false;
}

void nilou::AGeoreferenceActor::Deserialize(FArchive& Ar)
{
    nlohmann::json& Node = Ar.Node;
    nlohmann::json &content = Node["Content"];

    nilou::AActor::Deserialize(Ar);
}
