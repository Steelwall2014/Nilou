#include "D:/Nilou/src/Runtime/Framework/Common/Actor/ReflectionProbe.h"
#include "reflection/TypeDescriptorBuilder.h"
#include "reflection/Class.h"

using namespace nilou;
using namespace reflection;

std::unique_ptr<NClass> AReflectionProbe::StaticClass_ = nullptr;
const NClass *AReflectionProbe::GetClass() const 
{ 
    return AReflectionProbe::StaticClass(); 
}
const NClass *AReflectionProbe::StaticClass()
{
    return AReflectionProbe::StaticClass_.get();
}

template<>
struct TClassRegistry<AReflectionProbe>
{
    TClassRegistry(const std::string& InName)
    {
        AReflectionProbe::StaticClass_ = std::make_unique<NClass>();
        reflection::AddClass<AReflectionProbe>("AReflectionProbe")
				   .AddDefaultConstructor()
				   .AddParentClass("AActor")
;
        AReflectionProbe::StaticClass_->Type = reflection::Registry::GetTypeByName(InName);
    }

    static TClassRegistry<AReflectionProbe> Dummy;
};
TClassRegistry<AReflectionProbe> Dummy = TClassRegistry<AReflectionProbe>("AReflectionProbe");


