#include "D:/Nilou/src/Runtime/Framework/Common/Actor/CameraActor.h"
#include "reflection/TypeDescriptorBuilder.h"
#include "reflection/Class.h"

using namespace nilou;
using namespace reflection;

std::unique_ptr<NClass> ACameraActor::StaticClass_ = nullptr;
const NClass *ACameraActor::GetClass() const 
{ 
    return ACameraActor::StaticClass(); 
}
const NClass *ACameraActor::StaticClass()
{
    return ACameraActor::StaticClass_.get();
}

template<>
struct TClassRegistry<ACameraActor>
{
    TClassRegistry(const std::string& InName)
    {
        ACameraActor::StaticClass_ = std::make_unique<NClass>();
        reflection::AddClass<ACameraActor>("ACameraActor")
				   .AddDefaultConstructor()
				   .AddParentClass("AActor")
;
        ACameraActor::StaticClass_->Type = reflection::Registry::GetTypeByName(InName);
    }

    static TClassRegistry<ACameraActor> Dummy;
};
TClassRegistry<ACameraActor> Dummy = TClassRegistry<ACameraActor>("ACameraActor");


