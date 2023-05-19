#include "D:/Nilou/src/Runtime/Framework/Common/Actor/ReflectionProbe.h"
#include <UDRefl/UDRefl.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> nilou::AReflectionProbe::StaticClass_ = nullptr;
const NClass *nilou::AReflectionProbe::GetClass() const 
{ 
    return nilou::AReflectionProbe::StaticClass(); 
}
const NClass *nilou::AReflectionProbe::StaticClass()
{
    return nilou::AReflectionProbe::StaticClass_.get();
}

template<>
struct TClassRegistry<nilou::AReflectionProbe>
{
    TClassRegistry(const std::string& InName)
    {
        nilou::AReflectionProbe::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<nilou::AReflectionProbe>();
		Mngr.AddBases<nilou::AReflectionProbe, nilou::AActor>();
;
        nilou::AReflectionProbe::StaticClass_->Type = Type_of<nilou::AReflectionProbe>;
        nilou::AReflectionProbe::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<nilou::AReflectionProbe>);
    }

    static TClassRegistry<nilou::AReflectionProbe> Dummy;
};
TClassRegistry<nilou::AReflectionProbe> Dummy = TClassRegistry<nilou::AReflectionProbe>("nilou::AReflectionProbe");



void nilou::AReflectionProbe::Serialize(FArchive& Ar)
{
    nilou::AActor::Serialize(Ar);
    if (this->bIsSerializing)
        return;
    this->bIsSerializing = true;
    nlohmann::json& Node = Ar.Node;
    Node["ClassName"] = "nilou::AReflectionProbe";
    nlohmann::json &content = Node["Content"];

    this->bIsSerializing = false;
}

void nilou::AReflectionProbe::Deserialize(FArchive& Ar)
{
    nlohmann::json& Node = Ar.Node;
    nlohmann::json &content = Node["Content"];

    nilou::AActor::Deserialize(Ar);
}
