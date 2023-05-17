#include "D:/Nilou/src/Runtime/Framework/Common/Actor/FFTOceanActor.h"
#include <UDRefl/UDRefl.hpp>

using namespace nilou;
using namespace Ubpa;
using namespace Ubpa::UDRefl;

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
        Mngr.RegisterType<USceneCaptureComponentCube>();
		Mngr.AddConstructor<USceneCaptureComponentCube>();
		Mngr.AddBases<USceneCaptureComponentCube, USceneCaptureComponent>();
;
        USceneCaptureComponentCube::StaticClass_->Type = Type_of<USceneCaptureComponentCube>;
        USceneCaptureComponentCube::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<USceneCaptureComponentCube>);
    }

    static TClassRegistry<USceneCaptureComponentCube> Dummy;
};
TClassRegistry<USceneCaptureComponentCube> Dummy = TClassRegistry<USceneCaptureComponentCube>("USceneCaptureComponentCube");


