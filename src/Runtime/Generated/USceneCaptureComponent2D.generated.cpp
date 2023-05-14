#include "D:/Nilou/src/Runtime/Framework/Common/Actor/FFTOceanActor.h"
#include "reflection/TypeDescriptorBuilder.h"
#include "reflection/Class.h"

using namespace nilou;
using namespace reflection;

std::unique_ptr<NClass> USceneCaptureComponent2D::StaticClass_ = nullptr;
const NClass *USceneCaptureComponent2D::GetClass() const 
{ 
    return USceneCaptureComponent2D::StaticClass(); 
}
const NClass *USceneCaptureComponent2D::StaticClass()
{
    return USceneCaptureComponent2D::StaticClass_.get();
}

template<>
struct TClassRegistry<USceneCaptureComponent2D>
{
    TClassRegistry(const std::string& InName)
    {
        USceneCaptureComponent2D::StaticClass_ = std::make_unique<NClass>();
        reflection::AddClass<USceneCaptureComponent2D>("USceneCaptureComponent2D")
				   .AddDefaultConstructor()
				   .AddParentClass("USceneCaptureComponent")
;
        USceneCaptureComponent2D::StaticClass_->Type = reflection::Registry::GetTypeByName(InName);
    }

    static TClassRegistry<USceneCaptureComponent2D> Dummy;
};
TClassRegistry<USceneCaptureComponent2D> Dummy = TClassRegistry<USceneCaptureComponent2D>("USceneCaptureComponent2D");


