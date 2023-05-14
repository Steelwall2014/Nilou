#include "D:/Nilou/src/Runtime/Cesium3DTiles/Cesium3DTileComponent.h"
#include "reflection/TypeDescriptorBuilder.h"
#include "reflection/Class.h"

using namespace nilou;
using namespace reflection;

std::unique_ptr<NClass> UPrimitiveComponent::StaticClass_ = nullptr;
const NClass *UPrimitiveComponent::GetClass() const 
{ 
    return UPrimitiveComponent::StaticClass(); 
}
const NClass *UPrimitiveComponent::StaticClass()
{
    return UPrimitiveComponent::StaticClass_.get();
}

template<>
struct TClassRegistry<UPrimitiveComponent>
{
    TClassRegistry(const std::string& InName)
    {
        UPrimitiveComponent::StaticClass_ = std::make_unique<NClass>();
        reflection::AddClass<UPrimitiveComponent>("UPrimitiveComponent")
				   .AddDefaultConstructor()
				   .AddParentClass("USceneComponent")
				   .AddDerivedClass("UArrowComponent")
				   .AddDerivedClass("UCesium3DTileComponent")
				   .AddDerivedClass("UFourierTransformOceanComponent")
				   .AddDerivedClass("ULineBatchComponent")
				   .AddDerivedClass("USphereComponent")
				   .AddDerivedClass("UStaticMeshComponent")
				   .AddDerivedClass("UVirtualHeightfieldMeshComponent")
;
        UPrimitiveComponent::StaticClass_->Type = reflection::Registry::GetTypeByName(InName);
    }

    static TClassRegistry<UPrimitiveComponent> Dummy;
};
TClassRegistry<UPrimitiveComponent> Dummy = TClassRegistry<UPrimitiveComponent>("UPrimitiveComponent");


