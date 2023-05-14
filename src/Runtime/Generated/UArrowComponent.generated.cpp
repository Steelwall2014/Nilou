#include "D:/Nilou/src/Runtime/Framework/Common/Actor/ArrowActor.h"
#include "reflection/TypeDescriptorBuilder.h"
#include "reflection/Class.h"

using namespace nilou;
using namespace reflection;

std::unique_ptr<NClass> UArrowComponent::StaticClass_ = nullptr;
const NClass *UArrowComponent::GetClass() const 
{ 
    return UArrowComponent::StaticClass(); 
}
const NClass *UArrowComponent::StaticClass()
{
    return UArrowComponent::StaticClass_.get();
}

template<>
struct TClassRegistry<UArrowComponent>
{
    TClassRegistry(const std::string& InName)
    {
        UArrowComponent::StaticClass_ = std::make_unique<NClass>();
        reflection::AddClass<UArrowComponent>("UArrowComponent")
				   .AddDefaultConstructor()
				   .AddParentClass("UPrimitiveComponent")
;
        UArrowComponent::StaticClass_->Type = reflection::Registry::GetTypeByName(InName);
    }

    static TClassRegistry<UArrowComponent> Dummy;
};
TClassRegistry<UArrowComponent> Dummy = TClassRegistry<UArrowComponent>("UArrowComponent");


