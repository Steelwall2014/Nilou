#include "D:/Nilou/src/Runtime/Framework/Common/Components/VirtualHeightfieldMeshComponent.h"
#include <UDRefl/UDRefl.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> nilou::UMaterialInstance::StaticClass_ = nullptr;
const NClass *nilou::UMaterialInstance::GetClass() const 
{ 
    return nilou::UMaterialInstance::StaticClass(); 
}
const NClass *nilou::UMaterialInstance::StaticClass()
{
    return nilou::UMaterialInstance::StaticClass_.get();
}

template<>
struct TClassRegistry<nilou::UMaterialInstance>
{
    TClassRegistry(const std::string& InName)
    {
        nilou::UMaterialInstance::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<nilou::UMaterialInstance>();
		Mngr.AddBases<nilou::UMaterialInstance, nilou::UMaterial>();
;
        nilou::UMaterialInstance::StaticClass_->Type = Type_of<nilou::UMaterialInstance>;
        nilou::UMaterialInstance::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<nilou::UMaterialInstance>);
    }

    static TClassRegistry<nilou::UMaterialInstance> Dummy;
};
TClassRegistry<nilou::UMaterialInstance> Dummy = TClassRegistry<nilou::UMaterialInstance>("nilou::UMaterialInstance");



void nilou::UMaterialInstance::Serialize(FArchive& Ar)
{
    nilou::UMaterial::Serialize(Ar);
    if (this->bIsSerializing)
        return;
    this->bIsSerializing = true;
    nlohmann::json& Node = Ar.Node;
    Node["ClassName"] = "nilou::UMaterialInstance";
    nlohmann::json &content = Node["Content"];

    this->bIsSerializing = false;
}

void nilou::UMaterialInstance::Deserialize(FArchive& Ar)
{
    nlohmann::json& Node = Ar.Node;
    nlohmann::json &content = Node["Content"];

    nilou::UMaterial::Deserialize(Ar);
}
