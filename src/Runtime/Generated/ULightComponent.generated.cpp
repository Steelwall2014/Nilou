#include "D:/Nilou/src/Runtime/Framework/Common/Actor/FFTOceanActor.h"
#include <UDRefl/UDRefl.hpp>

using namespace nilou;
using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> ULightComponent::StaticClass_ = nullptr;
const NClass *ULightComponent::GetClass() const 
{ 
    return ULightComponent::StaticClass(); 
}
const NClass *ULightComponent::StaticClass()
{
    return ULightComponent::StaticClass_.get();
}

template<>
struct TClassRegistry<ULightComponent>
{
    TClassRegistry(const std::string& InName)
    {
        ULightComponent::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<ULightComponent>();
		Mngr.AddConstructor<ULightComponent>();
		Mngr.AddBases<ULightComponent, USceneComponent>();
;
        ULightComponent::StaticClass_->Type = Type_of<ULightComponent>;
        ULightComponent::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<ULightComponent>);
    }

    static TClassRegistry<ULightComponent> Dummy;
};
TClassRegistry<ULightComponent> Dummy = TClassRegistry<ULightComponent>("ULightComponent");


