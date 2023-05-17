#include "D:/Nilou/src/Runtime/Framework/Common/Actor/FFTOceanActor.h"
#include <UDRefl/UDRefl.hpp>

using namespace nilou;
using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> USceneCaptureComponent::StaticClass_ = nullptr;
const NClass *USceneCaptureComponent::GetClass() const 
{ 
    return USceneCaptureComponent::StaticClass(); 
}
const NClass *USceneCaptureComponent::StaticClass()
{
    return USceneCaptureComponent::StaticClass_.get();
}

template<>
struct TClassRegistry<USceneCaptureComponent>
{
    TClassRegistry(const std::string& InName)
    {
        USceneCaptureComponent::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<USceneCaptureComponent>();
		Mngr.AddConstructor<USceneCaptureComponent>();
		Mngr.AddBases<USceneCaptureComponent, USceneComponent>();
;
        USceneCaptureComponent::StaticClass_->Type = Type_of<USceneCaptureComponent>;
        USceneCaptureComponent::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<USceneCaptureComponent>);
    }

    static TClassRegistry<USceneCaptureComponent> Dummy;
};
TClassRegistry<USceneCaptureComponent> Dummy = TClassRegistry<USceneCaptureComponent>("USceneCaptureComponent");


