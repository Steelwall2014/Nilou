#include "D:/Nilou/src/Runtime/Framework/Common/Actor/FFTOceanActor.h"
#include "reflection/TypeDescriptorBuilder.h"
#include "reflection/Class.h"

using namespace nilou;
using namespace reflection;

std::unique_ptr<NClass> USceneCaptureComponent::StaticClass_ = nullptr;
const NClass *USceneCaptureComponent::GetClass() const 
{ 
    return USceneCaptureComponent::StaticClass(); 
}
const NClass *USceneCaptureComponent::StaticClass()
{
    return USceneCaptureComponent::StaticClass_.get();
}

template<>
struct TClassRegistry<USceneCaptureComponent>
{
    TClassRegistry(const std::string& InName)
    {
        USceneCaptureComponent::StaticClass_ = std::make_unique<NClass>();
        reflection::AddClass<USceneCaptureComponent>("USceneCaptureComponent")
				   .AddDefaultConstructor()
				   .AddParentClass("USceneComponent")
				   .AddDerivedClass("USceneCaptureComponent2D")
				   .AddDerivedClass("USceneCaptureComponentCube")
;
        USceneCaptureComponent::StaticClass_->Type = reflection::Registry::GetTypeByName(InName);
    }

    static TClassRegistry<USceneCaptureComponent> Dummy;
};
TClassRegistry<USceneCaptureComponent> Dummy = TClassRegistry<USceneCaptureComponent>("USceneCaptureComponent");


