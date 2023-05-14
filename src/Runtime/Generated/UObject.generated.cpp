#include "D:/Nilou/src/Runtime/Cesium3DTiles/Cesium3DTileComponent.h"
#include "reflection/TypeDescriptorBuilder.h"
#include "reflection/Class.h"

using namespace nilou;
using namespace reflection;

std::unique_ptr<NClass> UObject::StaticClass_ = nullptr;
const NClass *UObject::GetClass() const 
{ 
    return UObject::StaticClass(); 
}
const NClass *UObject::StaticClass()
{
    return UObject::StaticClass_.get();
}

template<>
struct TClassRegistry<UObject>
{
    TClassRegistry(const std::string& InName)
    {
        UObject::StaticClass_ = std::make_unique<NClass>();
        reflection::AddClass<UObject>("UObject")
				   .AddDefaultConstructor()
				   .AddDerivedClass("A")
				   .AddDerivedClass("AActor")
				   .AddDerivedClass("UActorComponent")
				   .AddDerivedClass("UMaterial")
				   .AddDerivedClass("UStaticMesh")
				   .AddDerivedClass("UTexture")
;
        UObject::StaticClass_->Type = reflection::Registry::GetTypeByName(InName);
    }

    static TClassRegistry<UObject> Dummy;
};
TClassRegistry<UObject> Dummy = TClassRegistry<UObject>("UObject");


