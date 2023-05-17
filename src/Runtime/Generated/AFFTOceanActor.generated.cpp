#include "D:/Nilou/src/Runtime/Framework/Common/Actor/FFTOceanActor.h"
#include <UDRefl/UDRefl.hpp>

using namespace nilou;
using namespace Ubpa;
using namespace Ubpa::UDRefl;

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
        Mngr.RegisterType<AFFTOceanActor>();
		Mngr.AddConstructor<AFFTOceanActor>();
		Mngr.AddBases<AFFTOceanActor, AActor>();
;
        AFFTOceanActor::StaticClass_->Type = Type_of<AFFTOceanActor>;
        AFFTOceanActor::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<AFFTOceanActor>);
    }

    static TClassRegistry<AFFTOceanActor> Dummy;
};
TClassRegistry<AFFTOceanActor> Dummy = TClassRegistry<AFFTOceanActor>("AFFTOceanActor");


