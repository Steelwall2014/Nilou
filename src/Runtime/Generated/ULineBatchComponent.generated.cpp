#include "D:/Nilou/src/Runtime/Framework/Common/Actor/LineBatchActor.h"
#include "reflection/TypeDescriptorBuilder.h"
#include "reflection/Class.h"

using namespace nilou;
using namespace reflection;

std::unique_ptr<NClass> ULineBatchComponent::StaticClass_ = nullptr;
const NClass *ULineBatchComponent::GetClass() const 
{ 
    return ULineBatchComponent::StaticClass(); 
}
const NClass *ULineBatchComponent::StaticClass()
{
    return ULineBatchComponent::StaticClass_.get();
}

template<>
struct TClassRegistry<ULineBatchComponent>
{
    TClassRegistry(const std::string& InName)
    {
        ULineBatchComponent::StaticClass_ = std::make_unique<NClass>();
        reflection::AddClass<ULineBatchComponent>("ULineBatchComponent")
				   .AddDefaultConstructor()
				   .AddParentClass("UPrimitiveComponent")
;
        ULineBatchComponent::StaticClass_->Type = reflection::Registry::GetTypeByName(InName);
    }

    static TClassRegistry<ULineBatchComponent> Dummy;
};
TClassRegistry<ULineBatchComponent> Dummy = TClassRegistry<ULineBatchComponent>("ULineBatchComponent");


