#include "D:/Nilou/src/Runtime/Cesium3DTiles/Cesium3DTileComponent.h"
#include "reflection/TypeDescriptorBuilder.h"
#include "reflection/Class.h"

using namespace nilou;
using namespace reflection;

std::unique_ptr<NClass> UCesium3DTileComponent::StaticClass_ = nullptr;
const NClass *UCesium3DTileComponent::GetClass() const 
{ 
    return UCesium3DTileComponent::StaticClass(); 
}
const NClass *UCesium3DTileComponent::StaticClass()
{
    return UCesium3DTileComponent::StaticClass_.get();
}

template<>
struct TClassRegistry<UCesium3DTileComponent>
{
    TClassRegistry(const std::string& InName)
    {
        UCesium3DTileComponent::StaticClass_ = std::make_unique<NClass>();
        reflection::AddClass<UCesium3DTileComponent>("UCesium3DTileComponent")
				   .AddDefaultConstructor()
				   .AddParentClass("UPrimitiveComponent")
;
        UCesium3DTileComponent::StaticClass_->Type = reflection::Registry::GetTypeByName(InName);
    }

    static TClassRegistry<UCesium3DTileComponent> Dummy;
};
TClassRegistry<UCesium3DTileComponent> Dummy = TClassRegistry<UCesium3DTileComponent>("UCesium3DTileComponent");


