#include "D:/Nilou/src/Runtime/Framework/Common/Actor/SkyAtmosphereActor.h"
#include <UDRefl/UDRefl.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> nilou::ASkyAtmosphereActor::StaticClass_ = nullptr;
const NClass *nilou::ASkyAtmosphereActor::GetClass() const 
{ 
    return nilou::ASkyAtmosphereActor::StaticClass(); 
}
const NClass *nilou::ASkyAtmosphereActor::StaticClass()
{
    return nilou::ASkyAtmosphereActor::StaticClass_.get();
}

template<>
struct TClassRegistry<nilou::ASkyAtmosphereActor>
{
    TClassRegistry(const std::string& InName)
    {
        nilou::ASkyAtmosphereActor::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<nilou::ASkyAtmosphereActor>();
		Mngr.AddBases<nilou::ASkyAtmosphereActor, nilou::AActor>();
;
        nilou::ASkyAtmosphereActor::StaticClass_->Type = Type_of<nilou::ASkyAtmosphereActor>;
        nilou::ASkyAtmosphereActor::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<nilou::ASkyAtmosphereActor>);
    }

    static TClassRegistry<nilou::ASkyAtmosphereActor> Dummy;
};
TClassRegistry<nilou::ASkyAtmosphereActor> Dummy = TClassRegistry<nilou::ASkyAtmosphereActor>("nilou::ASkyAtmosphereActor");



void nilou::ASkyAtmosphereActor::Serialize(FArchive& Ar)
{
    nilou::AActor::Serialize(Ar);
    if (this->bIsSerializing)
        return;
    this->bIsSerializing = true;
    nlohmann::json& Node = Ar.Node;
    Node["ClassName"] = "nilou::ASkyAtmosphereActor";
    nlohmann::json &content = Node["Content"];

    this->bIsSerializing = false;
}

void nilou::ASkyAtmosphereActor::Deserialize(FArchive& Ar)
{
    nlohmann::json& Node = Ar.Node;
    nlohmann::json &content = Node["Content"];

    nilou::AActor::Deserialize(Ar);
}
