#include "D:/Nilou/src/Runtime/Framework/Common/Actor/CameraActor.h"
#include <UDRefl/UDRefl.hpp>

using namespace nilou;
using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> UCameraComponent::StaticClass_ = nullptr;
const NClass *UCameraComponent::GetClass() const 
{ 
    return UCameraComponent::StaticClass(); 
}
const NClass *UCameraComponent::StaticClass()
{
    return UCameraComponent::StaticClass_.get();
}

template<>
struct TClassRegistry<UCameraComponent>
{
    TClassRegistry(const std::string& InName)
    {
        UCameraComponent::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<UCameraComponent>();
		Mngr.AddConstructor<UCameraComponent>();
		Mngr.AddBases<UCameraComponent, USceneComponent>();
;
        UCameraComponent::StaticClass_->Type = Type_of<UCameraComponent>;
        UCameraComponent::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<UCameraComponent>);
    }

    static TClassRegistry<UCameraComponent> Dummy;
};
TClassRegistry<UCameraComponent> Dummy = TClassRegistry<UCameraComponent>("UCameraComponent");


