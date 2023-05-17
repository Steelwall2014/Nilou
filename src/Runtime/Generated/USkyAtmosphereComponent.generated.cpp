#include "D:/Nilou/src/Runtime/Framework/Common/Actor/FFTOceanActor.h"
#include <UDRefl/UDRefl.hpp>

using namespace nilou;
using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> USkyAtmosphereComponent::StaticClass_ = nullptr;
const NClass *USkyAtmosphereComponent::GetClass() const 
{ 
    return USkyAtmosphereComponent::StaticClass(); 
}
const NClass *USkyAtmosphereComponent::StaticClass()
{
    return USkyAtmosphereComponent::StaticClass_.get();
}

template<>
struct TClassRegistry<USkyAtmosphereComponent>
{
    TClassRegistry(const std::string& InName)
    {
        USkyAtmosphereComponent::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<USkyAtmosphereComponent>();
		Mngr.AddConstructor<USkyAtmosphereComponent>();
		Mngr.AddBases<USkyAtmosphereComponent, USceneComponent>();
;
        USkyAtmosphereComponent::StaticClass_->Type = Type_of<USkyAtmosphereComponent>;
        USkyAtmosphereComponent::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<USkyAtmosphereComponent>);
    }

    static TClassRegistry<USkyAtmosphereComponent> Dummy;
};
TClassRegistry<USkyAtmosphereComponent> Dummy = TClassRegistry<USkyAtmosphereComponent>("USkyAtmosphereComponent");


