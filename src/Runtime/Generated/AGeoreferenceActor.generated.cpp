#include "D:/Nilou/src/Runtime/Cesium3DTiles/Cesium3DTileset.h"
#include "reflection/TypeDescriptorBuilder.h"
#include "reflection/Class.h"

using namespace nilou;
using namespace reflection;

std::unique_ptr<NClass> AGeoreferenceActor::StaticClass_ = nullptr;
const NClass *AGeoreferenceActor::GetClass() const 
{ 
    return AGeoreferenceActor::StaticClass(); 
}
const NClass *AGeoreferenceActor::StaticClass()
{
    return AGeoreferenceActor::StaticClass_.get();
}

template<>
struct TClassRegistry<AGeoreferenceActor>
{
    TClassRegistry(const std::string& InName)
    {
        AGeoreferenceActor::StaticClass_ = std::make_unique<NClass>();
        reflection::AddClass<AGeoreferenceActor>("AGeoreferenceActor")
				   .AddDefaultConstructor()
				   .AddParentClass("AActor")
;
        AGeoreferenceActor::StaticClass_->Type = reflection::Registry::GetTypeByName(InName);
    }

    static TClassRegistry<AGeoreferenceActor> Dummy;
};
TClassRegistry<AGeoreferenceActor> Dummy = TClassRegistry<AGeoreferenceActor>("AGeoreferenceActor");


