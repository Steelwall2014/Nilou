#include "D:/Nilou/src/Runtime/Framework/Common/Actor/FFTOceanActor.h"
#include <UDRefl/UDRefl.hpp>

using namespace nilou;
using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> UReflectionProbeComponent::StaticClass_ = nullptr;
const NClass *UReflectionProbeComponent::GetClass() const 
{ 
    return UReflectionProbeComponent::StaticClass(); 
}
const NClass *UReflectionProbeComponent::StaticClass()
{
    return UReflectionProbeComponent::StaticClass_.get();
}

template<>
struct TClassRegistry<UReflectionProbeComponent>
{
    TClassRegistry(const std::string& InName)
    {
        UReflectionProbeComponent::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<UReflectionProbeComponent>();
		Mngr.AddConstructor<UReflectionProbeComponent>();
		Mngr.AddBases<UReflectionProbeComponent, USceneCaptureComponentCube>();
;
        UReflectionProbeComponent::StaticClass_->Type = Type_of<UReflectionProbeComponent>;
        UReflectionProbeComponent::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<UReflectionProbeComponent>);
    }

    static TClassRegistry<UReflectionProbeComponent> Dummy;
};
TClassRegistry<UReflectionProbeComponent> Dummy = TClassRegistry<UReflectionProbeComponent>("UReflectionProbeComponent");


