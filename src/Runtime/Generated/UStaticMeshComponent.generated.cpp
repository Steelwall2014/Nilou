#include "D:/Nilou/src/Runtime/Framework/Common/Actor/StaticMeshActor.h"
#include <UDRefl/UDRefl.hpp>

using namespace nilou;
using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> UStaticMeshComponent::StaticClass_ = nullptr;
const NClass *UStaticMeshComponent::GetClass() const 
{ 
    return UStaticMeshComponent::StaticClass(); 
}
const NClass *UStaticMeshComponent::StaticClass()
{
    return UStaticMeshComponent::StaticClass_.get();
}

template<>
struct TClassRegistry<UStaticMeshComponent>
{
    TClassRegistry(const std::string& InName)
    {
        UStaticMeshComponent::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<UStaticMeshComponent>();
		Mngr.AddConstructor<UStaticMeshComponent>();
		Mngr.AddBases<UStaticMeshComponent, UPrimitiveComponent>();
;
        UStaticMeshComponent::StaticClass_->Type = Type_of<UStaticMeshComponent>;
        UStaticMeshComponent::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<UStaticMeshComponent>);
    }

    static TClassRegistry<UStaticMeshComponent> Dummy;
};
TClassRegistry<UStaticMeshComponent> Dummy = TClassRegistry<UStaticMeshComponent>("UStaticMeshComponent");


