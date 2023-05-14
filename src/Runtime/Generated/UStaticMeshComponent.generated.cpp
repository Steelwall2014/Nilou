#include "D:/Nilou/src/Runtime/Framework/Common/Actor/StaticMeshActor.h"
#include "reflection/TypeDescriptorBuilder.h"
#include "reflection/Class.h"

using namespace nilou;
using namespace reflection;

std::unique_ptr<NClass> UStaticMeshComponent::StaticClass_ = nullptr;
const NClass *UStaticMeshComponent::GetClass() const 
{ 
    return UStaticMeshComponent::StaticClass(); 
}
const NClass *UStaticMeshComponent::StaticClass()
{
    return UStaticMeshComponent::StaticClass_.get();
}

template<>
struct TClassRegistry<UStaticMeshComponent>
{
    TClassRegistry(const std::string& InName)
    {
        UStaticMeshComponent::StaticClass_ = std::make_unique<NClass>();
        reflection::AddClass<UStaticMeshComponent>("UStaticMeshComponent")
				   .AddDefaultConstructor()
				   .AddParentClass("UPrimitiveComponent")
;
        UStaticMeshComponent::StaticClass_->Type = reflection::Registry::GetTypeByName(InName);
    }

    static TClassRegistry<UStaticMeshComponent> Dummy;
};
TClassRegistry<UStaticMeshComponent> Dummy = TClassRegistry<UStaticMeshComponent>("UStaticMeshComponent");


