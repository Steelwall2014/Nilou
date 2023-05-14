#include "D:/Nilou/src/Runtime/Cesium3DTiles/Cesium3DTileComponent.h"
#include "reflection/TypeDescriptorBuilder.h"
#include "reflection/Class.h"

using namespace nilou;
using namespace reflection;

std::unique_ptr<NClass> UActorComponent::StaticClass_ = nullptr;
const NClass *UActorComponent::GetClass() const 
{ 
    return UActorComponent::StaticClass(); 
}
const NClass *UActorComponent::StaticClass()
{
    return UActorComponent::StaticClass_.get();
}

template<>
struct TClassRegistry<UActorComponent>
{
    TClassRegistry(const std::string& InName)
    {
        UActorComponent::StaticClass_ = std::make_unique<NClass>();
        reflection::AddClass<UActorComponent>("UActorComponent")
				   .AddDefaultConstructor()
				   .AddParentClass("UObject")
				   .AddDerivedClass("USceneComponent")
;
        UActorComponent::StaticClass_->Type = reflection::Registry::GetTypeByName(InName);
    }

    static TClassRegistry<UActorComponent> Dummy;
};
TClassRegistry<UActorComponent> Dummy = TClassRegistry<UActorComponent>("UActorComponent");


