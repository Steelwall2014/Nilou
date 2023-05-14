#include "D:/Nilou/src/Runtime/Framework/Common/Actor/CameraActor.h"
#include "reflection/TypeDescriptorBuilder.h"
#include "reflection/Class.h"

using namespace nilou;
using namespace reflection;

std::unique_ptr<NClass> UCameraComponent::StaticClass_ = nullptr;
const NClass *UCameraComponent::GetClass() const 
{ 
    return UCameraComponent::StaticClass(); 
}
const NClass *UCameraComponent::StaticClass()
{
    return UCameraComponent::StaticClass_.get();
}

template<>
struct TClassRegistry<UCameraComponent>
{
    TClassRegistry(const std::string& InName)
    {
        UCameraComponent::StaticClass_ = std::make_unique<NClass>();
        reflection::AddClass<UCameraComponent>("UCameraComponent")
				   .AddDefaultConstructor()
				   .AddParentClass("USceneComponent")
;
        UCameraComponent::StaticClass_->Type = reflection::Registry::GetTypeByName(InName);
    }

    static TClassRegistry<UCameraComponent> Dummy;
};
TClassRegistry<UCameraComponent> Dummy = TClassRegistry<UCameraComponent>("UCameraComponent");


