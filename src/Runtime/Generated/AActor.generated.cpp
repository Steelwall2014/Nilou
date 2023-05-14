#include "D:/Nilou/src/Runtime/Cesium3DTiles/Cesium3DTileset.h"
#include "reflection/TypeDescriptorBuilder.h"
#include "reflection/Class.h"

using namespace nilou;
using namespace reflection;

std::unique_ptr<NClass> AActor::StaticClass_ = nullptr;
const NClass *AActor::GetClass() const 
{ 
    return AActor::StaticClass(); 
}
const NClass *AActor::StaticClass()
{
    return AActor::StaticClass_.get();
}

template<>
struct TClassRegistry<AActor>
{
    TClassRegistry(const std::string& InName)
    {
        AActor::StaticClass_ = std::make_unique<NClass>();
        reflection::AddClass<AActor>("AActor")
				   .AddDefaultConstructor()
				   .AddMemberVariable("ActorName", &AActor::ActorName)
				   .AddParentClass("UObject")
				   .AddDerivedClass("AArrowActor")
				   .AddDerivedClass("ACameraActor")
				   .AddDerivedClass("ACesium3DTileset")
				   .AddDerivedClass("AFFTOceanActor")
				   .AddDerivedClass("AGeoreferenceActor")
				   .AddDerivedClass("ALightActor")
				   .AddDerivedClass("ALineBatchActor")
				   .AddDerivedClass("AReflectionProbe")
				   .AddDerivedClass("ASkyAtmosphereActor")
				   .AddDerivedClass("ASphereActor")
				   .AddDerivedClass("AStaticMeshActor")
				   .AddDerivedClass("AVirtualHeightfieldMeshActor")
;
        AActor::StaticClass_->Type = reflection::Registry::GetTypeByName(InName);
    }

    static TClassRegistry<AActor> Dummy;
};
TClassRegistry<AActor> Dummy = TClassRegistry<AActor>("AActor");


