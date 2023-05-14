#include "D:/Nilou/src/Runtime/Framework/Common/Actor/VirtualHeightfieldMeshActor.h"
#include "reflection/TypeDescriptorBuilder.h"
#include "reflection/Class.h"

using namespace nilou;
using namespace reflection;

std::unique_ptr<NClass> AVirtualHeightfieldMeshActor::StaticClass_ = nullptr;
const NClass *AVirtualHeightfieldMeshActor::GetClass() const 
{ 
    return AVirtualHeightfieldMeshActor::StaticClass(); 
}
const NClass *AVirtualHeightfieldMeshActor::StaticClass()
{
    return AVirtualHeightfieldMeshActor::StaticClass_.get();
}

template<>
struct TClassRegistry<AVirtualHeightfieldMeshActor>
{
    TClassRegistry(const std::string& InName)
    {
        AVirtualHeightfieldMeshActor::StaticClass_ = std::make_unique<NClass>();
        reflection::AddClass<AVirtualHeightfieldMeshActor>("AVirtualHeightfieldMeshActor")
				   .AddDefaultConstructor()
				   .AddParentClass("AActor")
;
        AVirtualHeightfieldMeshActor::StaticClass_->Type = reflection::Registry::GetTypeByName(InName);
    }

    static TClassRegistry<AVirtualHeightfieldMeshActor> Dummy;
};
TClassRegistry<AVirtualHeightfieldMeshActor> Dummy = TClassRegistry<AVirtualHeightfieldMeshActor>("AVirtualHeightfieldMeshActor");


