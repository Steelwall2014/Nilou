#include "D:/Nilou/src/Runtime/Framework/Common/Actor/LineBatchActor.h"
#include "reflection/TypeDescriptorBuilder.h"
#include "reflection/Class.h"

using namespace nilou;
using namespace reflection;

std::unique_ptr<NClass> ALineBatchActor::StaticClass_ = nullptr;
const NClass *ALineBatchActor::GetClass() const 
{ 
    return ALineBatchActor::StaticClass(); 
}
const NClass *ALineBatchActor::StaticClass()
{
    return ALineBatchActor::StaticClass_.get();
}

template<>
struct TClassRegistry<ALineBatchActor>
{
    TClassRegistry(const std::string& InName)
    {
        ALineBatchActor::StaticClass_ = std::make_unique<NClass>();
        reflection::AddClass<ALineBatchActor>("ALineBatchActor")
				   .AddDefaultConstructor()
				   .AddParentClass("AActor")
;
        ALineBatchActor::StaticClass_->Type = reflection::Registry::GetTypeByName(InName);
    }

    static TClassRegistry<ALineBatchActor> Dummy;
};
TClassRegistry<ALineBatchActor> Dummy = TClassRegistry<ALineBatchActor>("ALineBatchActor");


