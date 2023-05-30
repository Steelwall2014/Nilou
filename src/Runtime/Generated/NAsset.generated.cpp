#include "D:/Nilou/src/Runtime/Framework/Common/Actor/Actor.h"
#include <UDRefl/UDRefl.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> nilou::NAsset::StaticClass_ = nullptr;
const NClass *nilou::NAsset::GetClass() const 
{ 
    return nilou::NAsset::StaticClass(); 
}
const NClass *nilou::NAsset::StaticClass()
{
    return nilou::NAsset::StaticClass_.get();
}

template<>
struct TClassRegistry<nilou::NAsset>
{
    TClassRegistry(const std::string& InName)
    {
        nilou::NAsset::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<nilou::NAsset>();
		Mngr.AddBases<nilou::NAsset, NObject>();
;
        nilou::NAsset::StaticClass_->Type = Type_of<nilou::NAsset>;
        nilou::NAsset::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<nilou::NAsset>);
    }

    static TClassRegistry<nilou::NAsset> Dummy;
};
TClassRegistry<nilou::NAsset> Dummy = TClassRegistry<nilou::NAsset>("nilou::NAsset");



void nilou::NAsset::Serialize(FArchive& Ar)
{
    NObject::Serialize(Ar);
    if (this->bIsSerializing)
        return;
    this->bIsSerializing = true;
    nlohmann::json& Node = Ar.Node;
    Node["ClassName"] = "nilou::NAsset";
    nlohmann::json &content = Node["Content"];

    this->bIsSerializing = false;
}

void nilou::NAsset::Deserialize(FArchive& Ar)
{
    nlohmann::json& Node = Ar.Node;
    nlohmann::json &content = Node["Content"];

    NObject::Deserialize(Ar);
}
