#include "D:/Nilou/src/Runtime/Framework/Common/Actor/VirtualHeightfieldMeshActor.h"
#include <UDRefl/UDRefl.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> nilou::UVirtualHeightfieldMeshComponent::StaticClass_ = nullptr;
const NClass *nilou::UVirtualHeightfieldMeshComponent::GetClass() const 
{ 
    return nilou::UVirtualHeightfieldMeshComponent::StaticClass(); 
}
const NClass *nilou::UVirtualHeightfieldMeshComponent::StaticClass()
{
    return nilou::UVirtualHeightfieldMeshComponent::StaticClass_.get();
}

template<>
struct TClassRegistry<nilou::UVirtualHeightfieldMeshComponent>
{
    TClassRegistry(const std::string& InName)
    {
        nilou::UVirtualHeightfieldMeshComponent::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<nilou::UVirtualHeightfieldMeshComponent>();
		Mngr.AddBases<nilou::UVirtualHeightfieldMeshComponent, nilou::UPrimitiveComponent>();
;
        nilou::UVirtualHeightfieldMeshComponent::StaticClass_->Type = Type_of<nilou::UVirtualHeightfieldMeshComponent>;
        nilou::UVirtualHeightfieldMeshComponent::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<nilou::UVirtualHeightfieldMeshComponent>);
    }

    static TClassRegistry<nilou::UVirtualHeightfieldMeshComponent> Dummy;
};
TClassRegistry<nilou::UVirtualHeightfieldMeshComponent> Dummy = TClassRegistry<nilou::UVirtualHeightfieldMeshComponent>("nilou::UVirtualHeightfieldMeshComponent");



void nilou::UVirtualHeightfieldMeshComponent::Serialize(FArchive& Ar)
{
    nilou::UPrimitiveComponent::Serialize(Ar);
    if (this->bIsSerializing)
        return;
    this->bIsSerializing = true;
    nlohmann::json& Node = Ar.Node;
    Node["ClassName"] = "nilou::UVirtualHeightfieldMeshComponent";
    nlohmann::json &content = Node["Content"];

    this->bIsSerializing = false;
}

void nilou::UVirtualHeightfieldMeshComponent::Deserialize(FArchive& Ar)
{
    nlohmann::json& Node = Ar.Node;
    nlohmann::json &content = Node["Content"];

    nilou::UPrimitiveComponent::Deserialize(Ar);
}
