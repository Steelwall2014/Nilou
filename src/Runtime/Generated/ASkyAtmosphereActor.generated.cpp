#include "D:/Nilou/src/Runtime/Framework/Common/Actor/SkyAtmosphereActor.h"
#include "reflection/TypeDescriptorBuilder.h"
#include "reflection/Class.h"

using namespace nilou;
using namespace reflection;

std::unique_ptr<NClass> ASkyAtmosphereActor::StaticClass_ = nullptr;
const NClass *ASkyAtmosphereActor::GetClass() const 
{ 
    return ASkyAtmosphereActor::StaticClass(); 
}
const NClass *ASkyAtmosphereActor::StaticClass()
{
    return ASkyAtmosphereActor::StaticClass_.get();
}

template<>
struct TClassRegistry<ASkyAtmosphereActor>
{
    TClassRegistry(const std::string& InName)
    {
        ASkyAtmosphereActor::StaticClass_ = std::make_unique<NClass>();
        reflection::AddClass<ASkyAtmosphereActor>("ASkyAtmosphereActor")
				   .AddDefaultConstructor()
				   .AddParentClass("AActor")
;
        ASkyAtmosphereActor::StaticClass_->Type = reflection::Registry::GetTypeByName(InName);
    }

    static TClassRegistry<ASkyAtmosphereActor> Dummy;
};
TClassRegistry<ASkyAtmosphereActor> Dummy = TClassRegistry<ASkyAtmosphereActor>("ASkyAtmosphereActor");


