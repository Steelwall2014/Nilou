#include "D:/Nilou/src/Runtime/Framework/Common/Actor/FFTOceanActor.h"
#include "reflection/TypeDescriptorBuilder.h"
#include "reflection/Class.h"

using namespace nilou;
using namespace reflection;

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
        reflection::AddClass<ULightComponent>("ULightComponent")
				   .AddDefaultConstructor()
				   .AddParentClass("USceneComponent")
;
        ULightComponent::StaticClass_->Type = reflection::Registry::GetTypeByName(InName);
    }

    static TClassRegistry<ULightComponent> Dummy;
};
TClassRegistry<ULightComponent> Dummy = TClassRegistry<ULightComponent>("ULightComponent");


