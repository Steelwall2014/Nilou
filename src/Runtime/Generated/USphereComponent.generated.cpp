#include "D:/Nilou/src/Runtime/Framework/Common/Actor/SphereActor.h"
#include "reflection/TypeDescriptorBuilder.h"
#include "reflection/Class.h"

using namespace nilou;
using namespace reflection;

std::unique_ptr<NClass> USphereComponent::StaticClass_ = nullptr;
const NClass *USphereComponent::GetClass() const 
{ 
    return USphereComponent::StaticClass(); 
}
const NClass *USphereComponent::StaticClass()
{
    return USphereComponent::StaticClass_.get();
}

template<>
struct TClassRegistry<USphereComponent>
{
    TClassRegistry(const std::string& InName)
    {
        USphereComponent::StaticClass_ = std::make_unique<NClass>();
        reflection::AddClass<USphereComponent>("USphereComponent")
				   .AddDefaultConstructor()
				   .AddParentClass("UPrimitiveComponent")
;
        USphereComponent::StaticClass_->Type = reflection::Registry::GetTypeByName(InName);
    }

    static TClassRegistry<USphereComponent> Dummy;
};
TClassRegistry<USphereComponent> Dummy = TClassRegistry<USphereComponent>("USphereComponent");


