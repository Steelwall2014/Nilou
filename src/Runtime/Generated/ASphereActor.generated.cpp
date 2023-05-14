#include "D:/Nilou/src/Runtime/Framework/Common/Actor/SphereActor.h"
#include "reflection/TypeDescriptorBuilder.h"
#include "reflection/Class.h"

using namespace nilou;
using namespace reflection;

std::unique_ptr<NClass> ASphereActor::StaticClass_ = nullptr;
const NClass *ASphereActor::GetClass() const 
{ 
    return ASphereActor::StaticClass(); 
}
const NClass *ASphereActor::StaticClass()
{
    return ASphereActor::StaticClass_.get();
}

template<>
struct TClassRegistry<ASphereActor>
{
    TClassRegistry(const std::string& InName)
    {
        ASphereActor::StaticClass_ = std::make_unique<NClass>();
        reflection::AddClass<ASphereActor>("ASphereActor")
				   .AddDefaultConstructor()
				   .AddParentClass("AActor")
;
        ASphereActor::StaticClass_->Type = reflection::Registry::GetTypeByName(InName);
    }

    static TClassRegistry<ASphereActor> Dummy;
};
TClassRegistry<ASphereActor> Dummy = TClassRegistry<ASphereActor>("ASphereActor");


