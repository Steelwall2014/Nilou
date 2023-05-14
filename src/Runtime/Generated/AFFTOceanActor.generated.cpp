#include "D:/Nilou/src/Runtime/Framework/Common/Actor/FFTOceanActor.h"
#include "reflection/TypeDescriptorBuilder.h"
#include "reflection/Class.h"

using namespace nilou;
using namespace reflection;

std::unique_ptr<NClass> AFFTOceanActor::StaticClass_ = nullptr;
const NClass *AFFTOceanActor::GetClass() const 
{ 
    return AFFTOceanActor::StaticClass(); 
}
const NClass *AFFTOceanActor::StaticClass()
{
    return AFFTOceanActor::StaticClass_.get();
}

template<>
struct TClassRegistry<AFFTOceanActor>
{
    TClassRegistry(const std::string& InName)
    {
        AFFTOceanActor::StaticClass_ = std::make_unique<NClass>();
        reflection::AddClass<AFFTOceanActor>("AFFTOceanActor")
				   .AddDefaultConstructor()
				   .AddParentClass("AActor")
;
        AFFTOceanActor::StaticClass_->Type = reflection::Registry::GetTypeByName(InName);
    }

    static TClassRegistry<AFFTOceanActor> Dummy;
};
TClassRegistry<AFFTOceanActor> Dummy = TClassRegistry<AFFTOceanActor>("AFFTOceanActor");


