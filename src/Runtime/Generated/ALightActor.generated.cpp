#include "D:/Nilou/src/Runtime/Framework/Common/Actor/LightActor.h"
#include "reflection/TypeDescriptorBuilder.h"
#include "reflection/Class.h"

using namespace nilou;
using namespace reflection;

std::unique_ptr<NClass> ALightActor::StaticClass_ = nullptr;
const NClass *ALightActor::GetClass() const 
{ 
    return ALightActor::StaticClass(); 
}
const NClass *ALightActor::StaticClass()
{
    return ALightActor::StaticClass_.get();
}

template<>
struct TClassRegistry<ALightActor>
{
    TClassRegistry(const std::string& InName)
    {
        ALightActor::StaticClass_ = std::make_unique<NClass>();
        reflection::AddClass<ALightActor>("ALightActor")
				   .AddDefaultConstructor()
				   .AddParentClass("AActor")
;
        ALightActor::StaticClass_->Type = reflection::Registry::GetTypeByName(InName);
    }

    static TClassRegistry<ALightActor> Dummy;
};
TClassRegistry<ALightActor> Dummy = TClassRegistry<ALightActor>("ALightActor");


