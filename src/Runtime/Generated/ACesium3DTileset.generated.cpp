#include "D:/Nilou/src/Runtime/Cesium3DTiles/Cesium3DTileset.h"
#include <UDRefl/UDRefl.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> nilou::ACesium3DTileset::StaticClass_ = nullptr;
const NClass *nilou::ACesium3DTileset::GetClass() const 
{ 
    return nilou::ACesium3DTileset::StaticClass(); 
}
const NClass *nilou::ACesium3DTileset::StaticClass()
{
    return nilou::ACesium3DTileset::StaticClass_.get();
}

template<>
struct TClassRegistry<nilou::ACesium3DTileset>
{
    TClassRegistry(const std::string& InName)
    {
        nilou::ACesium3DTileset::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<nilou::ACesium3DTileset>();
		Mngr.AddBases<nilou::ACesium3DTileset, nilou::AActor>();
;
        nilou::ACesium3DTileset::StaticClass_->Type = Type_of<nilou::ACesium3DTileset>;
        nilou::ACesium3DTileset::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<nilou::ACesium3DTileset>);
    }

    static TClassRegistry<nilou::ACesium3DTileset> Dummy;
};
TClassRegistry<nilou::ACesium3DTileset> Dummy = TClassRegistry<nilou::ACesium3DTileset>("nilou::ACesium3DTileset");



void nilou::ACesium3DTileset::Serialize(FArchive& Ar)
{
    nilou::AActor::Serialize(Ar);
    if (this->bIsSerializing)
        return;
    this->bIsSerializing = true;
    nlohmann::json& Node = Ar.Node;
    Node["ClassName"] = "nilou::ACesium3DTileset";
    nlohmann::json &content = Node["Content"];

    this->bIsSerializing = false;
}

void nilou::ACesium3DTileset::Deserialize(FArchive& Ar)
{
    nlohmann::json& Node = Ar.Node;
    nlohmann::json &content = Node["Content"];

    nilou::AActor::Deserialize(Ar);
}
