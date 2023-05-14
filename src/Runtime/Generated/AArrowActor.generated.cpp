#include "D:/Nilou/src/Runtime/Framework/Common/Actor/ArrowActor.h"
#include "reflection/TypeDescriptorBuilder.h"
#include "reflection/Class.h"

using namespace nilou;
using namespace reflection;

std::unique_ptr<NClass> AArrowActor::StaticClass_ = nullptr;
const NClass *AArrowActor::GetClass() const 
{ 
    return AArrowActor::StaticClass(); 
}
const NClass *AArrowActor::StaticClass()
{
    return AArrowActor::StaticClass_.get();
}

template<>
struct TClassRegistry<AArrowActor>
{
    TClassRegistry(const std::string& InName)
    {
        AArrowActor::StaticClass_ = std::make_unique<NClass>();
        reflection::AddClass<AArrowActor>("AArrowActor")
				   .AddDefaultConstructor()
				   .AddParentClass("AActor")
;
        AArrowActor::StaticClass_->Type = reflection::Registry::GetTypeByName(InName);
    }

    static TClassRegistry<AArrowActor> Dummy;
};
TClassRegistry<AArrowActor> Dummy = TClassRegistry<AArrowActor>("AArrowActor");


