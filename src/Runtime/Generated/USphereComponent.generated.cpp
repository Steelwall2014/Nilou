#include "D:/Nilou/src/Runtime/Framework/Common/Actor/SphereActor.h"
#include <UDRefl/UDRefl.hpp>

using namespace nilou;
using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> USphereComponent::StaticClass_ = nullptr;
const NClass *USphereComponent::GetClass() const 
{ 
    return USphereComponent::StaticClass(); 
}
const NClass *USphereComponent::StaticClass()
{
    return USphereComponent::StaticClass_.get();
}

template<>
struct TClassRegistry<USphereComponent>
{
    TClassRegistry(const std::string& InName)
    {
        USphereComponent::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<USphereComponent>();
		Mngr.AddConstructor<USphereComponent>();
		Mngr.AddBases<USphereComponent, UPrimitiveComponent>();
;
        USphereComponent::StaticClass_->Type = Type_of<USphereComponent>;
        USphereComponent::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<USphereComponent>);
    }

    static TClassRegistry<USphereComponent> Dummy;
};
TClassRegistry<USphereComponent> Dummy = TClassRegistry<USphereComponent>("USphereComponent");


