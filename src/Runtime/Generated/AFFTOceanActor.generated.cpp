#include "D:/Nilou/src/Runtime/Framework/Common/Actor/FFTOceanActor.h"
#include <UDRefl/UDRefl.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> nilou::AFFTOceanActor::StaticClass_ = nullptr;
const NClass *nilou::AFFTOceanActor::GetClass() const 
{ 
    return nilou::AFFTOceanActor::StaticClass(); 
}
const NClass *nilou::AFFTOceanActor::StaticClass()
{
    return nilou::AFFTOceanActor::StaticClass_.get();
}

template<>
struct TClassRegistry<nilou::AFFTOceanActor>
{
    TClassRegistry(const std::string& InName)
    {
        nilou::AFFTOceanActor::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<nilou::AFFTOceanActor>();
		Mngr.AddBases<nilou::AFFTOceanActor, nilou::AActor>();
;
        nilou::AFFTOceanActor::StaticClass_->Type = Type_of<nilou::AFFTOceanActor>;
        nilou::AFFTOceanActor::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<nilou::AFFTOceanActor>);
    }

    static TClassRegistry<nilou::AFFTOceanActor> Dummy;
};
TClassRegistry<nilou::AFFTOceanActor> Dummy = TClassRegistry<nilou::AFFTOceanActor>("nilou::AFFTOceanActor");



void nilou::AFFTOceanActor::Serialize(FArchive& Ar)
{
    nilou::AActor::Serialize(Ar);
    if (this->bIsSerializing)
        return;
    this->bIsSerializing = true;
    nlohmann::json& Node = Ar.Node;
    Node["ClassName"] = "nilou::AFFTOceanActor";
    nlohmann::json &content = Node["Content"];

    this->bIsSerializing = false;
}

void nilou::AFFTOceanActor::Deserialize(FArchive& Ar)
{
    nlohmann::json& Node = Ar.Node;
    nlohmann::json &content = Node["Content"];

    nilou::AActor::Deserialize(Ar);
}
