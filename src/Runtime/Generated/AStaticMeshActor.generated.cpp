#include "D:/Nilou/src/Runtime/Framework/Common/Actor/StaticMeshActor.h"
#include "reflection/TypeDescriptorBuilder.h"
#include "reflection/Class.h"

using namespace nilou;
using namespace reflection;

std::unique_ptr<NClass> AStaticMeshActor::StaticClass_ = nullptr;
const NClass *AStaticMeshActor::GetClass() const 
{ 
    return AStaticMeshActor::StaticClass(); 
}
const NClass *AStaticMeshActor::StaticClass()
{
    return AStaticMeshActor::StaticClass_.get();
}

template<>
struct TClassRegistry<AStaticMeshActor>
{
    TClassRegistry(const std::string& InName)
    {
        AStaticMeshActor::StaticClass_ = std::make_unique<NClass>();
        reflection::AddClass<AStaticMeshActor>("AStaticMeshActor")
				   .AddDefaultConstructor()
				   .AddParentClass("AActor")
;
        AStaticMeshActor::StaticClass_->Type = reflection::Registry::GetTypeByName(InName);
    }

    static TClassRegistry<AStaticMeshActor> Dummy;
};
TClassRegistry<AStaticMeshActor> Dummy = TClassRegistry<AStaticMeshActor>("AStaticMeshActor");


