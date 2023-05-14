#include "D:/Nilou/src/Runtime/Framework/Common/Actor/VirtualHeightfieldMeshActor.h"
#include "reflection/TypeDescriptorBuilder.h"
#include "reflection/Class.h"

using namespace nilou;
using namespace reflection;

std::unique_ptr<NClass> UVirtualHeightfieldMeshComponent::StaticClass_ = nullptr;
const NClass *UVirtualHeightfieldMeshComponent::GetClass() const 
{ 
    return UVirtualHeightfieldMeshComponent::StaticClass(); 
}
const NClass *UVirtualHeightfieldMeshComponent::StaticClass()
{
    return UVirtualHeightfieldMeshComponent::StaticClass_.get();
}

template<>
struct TClassRegistry<UVirtualHeightfieldMeshComponent>
{
    TClassRegistry(const std::string& InName)
    {
        UVirtualHeightfieldMeshComponent::StaticClass_ = std::make_unique<NClass>();
        reflection::AddClass<UVirtualHeightfieldMeshComponent>("UVirtualHeightfieldMeshComponent")
				   .AddDefaultConstructor()
				   .AddParentClass("UPrimitiveComponent")
;
        UVirtualHeightfieldMeshComponent::StaticClass_->Type = reflection::Registry::GetTypeByName(InName);
    }

    static TClassRegistry<UVirtualHeightfieldMeshComponent> Dummy;
};
TClassRegistry<UVirtualHeightfieldMeshComponent> Dummy = TClassRegistry<UVirtualHeightfieldMeshComponent>("UVirtualHeightfieldMeshComponent");


