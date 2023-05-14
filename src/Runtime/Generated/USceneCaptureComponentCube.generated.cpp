#include "D:/Nilou/src/Runtime/Framework/Common/Actor/FFTOceanActor.h"
#include "reflection/TypeDescriptorBuilder.h"
#include "reflection/Class.h"

using namespace nilou;
using namespace reflection;

std::unique_ptr<NClass> USceneCaptureComponentCube::StaticClass_ = nullptr;
const NClass *USceneCaptureComponentCube::GetClass() const 
{ 
    return USceneCaptureComponentCube::StaticClass(); 
}
const NClass *USceneCaptureComponentCube::StaticClass()
{
    return USceneCaptureComponentCube::StaticClass_.get();
}

template<>
struct TClassRegistry<USceneCaptureComponentCube>
{
    TClassRegistry(const std::string& InName)
    {
        USceneCaptureComponentCube::StaticClass_ = std::make_unique<NClass>();
        reflection::AddClass<USceneCaptureComponentCube>("USceneCaptureComponentCube")
				   .AddDefaultConstructor()
				   .AddParentClass("USceneCaptureComponent")
				   .AddDerivedClass("UReflectionProbeComponent")
;
        USceneCaptureComponentCube::StaticClass_->Type = reflection::Registry::GetTypeByName(InName);
    }

    static TClassRegistry<USceneCaptureComponentCube> Dummy;
};
TClassRegistry<USceneCaptureComponentCube> Dummy = TClassRegistry<USceneCaptureComponentCube>("USceneCaptureComponentCube");


