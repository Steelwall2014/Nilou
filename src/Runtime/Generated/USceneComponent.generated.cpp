#include "D:/Nilou/src/Runtime/Cesium3DTiles/Cesium3DTileComponent.h"
#include "reflection/TypeDescriptorBuilder.h"
#include "reflection/Class.h"

using namespace nilou;
using namespace reflection;

std::unique_ptr<NClass> USceneComponent::StaticClass_ = nullptr;
const NClass *USceneComponent::GetClass() const 
{ 
    return USceneComponent::StaticClass(); 
}
const NClass *USceneComponent::StaticClass()
{
    return USceneComponent::StaticClass_.get();
}

template<>
struct TClassRegistry<USceneComponent>
{
    TClassRegistry(const std::string& InName)
    {
        USceneComponent::StaticClass_ = std::make_unique<NClass>();
        reflection::AddClass<USceneComponent>("USceneComponent")
				   .AddDefaultConstructor()
				   .AddParentClass("UActorComponent")
				   .AddDerivedClass("UCameraComponent")
				   .AddDerivedClass("ULightComponent")
				   .AddDerivedClass("UPrimitiveComponent")
				   .AddDerivedClass("USceneCaptureComponent")
				   .AddDerivedClass("USkyAtmosphereComponent")
;
        USceneComponent::StaticClass_->Type = reflection::Registry::GetTypeByName(InName);
    }

    static TClassRegistry<USceneComponent> Dummy;
};
TClassRegistry<USceneComponent> Dummy = TClassRegistry<USceneComponent>("USceneComponent");


