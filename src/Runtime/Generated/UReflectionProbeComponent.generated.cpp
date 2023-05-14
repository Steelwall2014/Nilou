#include "D:/Nilou/src/Runtime/Framework/Common/Actor/FFTOceanActor.h"
#include "reflection/TypeDescriptorBuilder.h"
#include "reflection/Class.h"

using namespace nilou;
using namespace reflection;

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
        reflection::AddClass<UReflectionProbeComponent>("UReflectionProbeComponent")
				   .AddDefaultConstructor()
				   .AddParentClass("USceneCaptureComponentCube")
;
        UReflectionProbeComponent::StaticClass_->Type = reflection::Registry::GetTypeByName(InName);
    }

    static TClassRegistry<UReflectionProbeComponent> Dummy;
};
TClassRegistry<UReflectionProbeComponent> Dummy = TClassRegistry<UReflectionProbeComponent>("UReflectionProbeComponent");


