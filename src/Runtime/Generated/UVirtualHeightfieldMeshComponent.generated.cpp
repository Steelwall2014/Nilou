#include "D:/Nilou/src/Runtime/Framework/Common/Actor/VirtualHeightfieldMeshActor.h"
#include <UDRefl/UDRefl.hpp>

using namespace nilou;
using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> UVirtualHeightfieldMeshComponent::StaticClass_ = nullptr;
const NClass *UVirtualHeightfieldMeshComponent::GetClass() const 
{ 
    return UVirtualHeightfieldMeshComponent::StaticClass(); 
}
const NClass *UVirtualHeightfieldMeshComponent::StaticClass()
{
    return UVirtualHeightfieldMeshComponent::StaticClass_.get();
}

template<>
struct TClassRegistry<UVirtualHeightfieldMeshComponent>
{
    TClassRegistry(const std::string& InName)
    {
        UVirtualHeightfieldMeshComponent::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<UVirtualHeightfieldMeshComponent>();
		Mngr.AddConstructor<UVirtualHeightfieldMeshComponent>();
		Mngr.AddBases<UVirtualHeightfieldMeshComponent, UPrimitiveComponent>();
;
        UVirtualHeightfieldMeshComponent::StaticClass_->Type = Type_of<UVirtualHeightfieldMeshComponent>;
        UVirtualHeightfieldMeshComponent::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<UVirtualHeightfieldMeshComponent>);
    }

    static TClassRegistry<UVirtualHeightfieldMeshComponent> Dummy;
};
TClassRegistry<UVirtualHeightfieldMeshComponent> Dummy = TClassRegistry<UVirtualHeightfieldMeshComponent>("UVirtualHeightfieldMeshComponent");


