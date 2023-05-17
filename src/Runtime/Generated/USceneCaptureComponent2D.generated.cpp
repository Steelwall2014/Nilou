#include "D:/Nilou/src/Runtime/Framework/Common/Actor/FFTOceanActor.h"
#include <UDRefl/UDRefl.hpp>

using namespace nilou;
using namespace Ubpa;
using namespace Ubpa::UDRefl;

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
        Mngr.RegisterType<USceneCaptureComponent2D>();
		Mngr.AddConstructor<USceneCaptureComponent2D>();
		Mngr.AddBases<USceneCaptureComponent2D, USceneCaptureComponent>();
;
        USceneCaptureComponent2D::StaticClass_->Type = Type_of<USceneCaptureComponent2D>;
        USceneCaptureComponent2D::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<USceneCaptureComponent2D>);
    }

    static TClassRegistry<USceneCaptureComponent2D> Dummy;
};
TClassRegistry<USceneCaptureComponent2D> Dummy = TClassRegistry<USceneCaptureComponent2D>("USceneCaptureComponent2D");


