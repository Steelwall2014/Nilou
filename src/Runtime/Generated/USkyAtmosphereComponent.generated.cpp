#include "D:/Nilou/src/Runtime/Framework/Common/Actor/FFTOceanActor.h"
#include "reflection/TypeDescriptorBuilder.h"
#include "reflection/Class.h"

using namespace nilou;
using namespace reflection;

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
        reflection::AddClass<USkyAtmosphereComponent>("USkyAtmosphereComponent")
				   .AddDefaultConstructor()
				   .AddParentClass("USceneComponent")
;
        USkyAtmosphereComponent::StaticClass_->Type = reflection::Registry::GetTypeByName(InName);
    }

    static TClassRegistry<USkyAtmosphereComponent> Dummy;
};
TClassRegistry<USkyAtmosphereComponent> Dummy = TClassRegistry<USkyAtmosphereComponent>("USkyAtmosphereComponent");


