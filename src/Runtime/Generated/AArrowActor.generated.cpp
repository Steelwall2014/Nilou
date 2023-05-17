#include "D:/Nilou/src/Runtime/Framework/Common/Actor/ArrowActor.h"
#include <UDRefl/UDRefl.hpp>

using namespace nilou;
using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> AArrowActor::StaticClass_ = nullptr;
const NClass *AArrowActor::GetClass() const 
{ 
    return AArrowActor::StaticClass(); 
}
const NClass *AArrowActor::StaticClass()
{
    return AArrowActor::StaticClass_.get();
}

template<>
struct TClassRegistry<AArrowActor>
{
    TClassRegistry(const std::string& InName)
    {
        AArrowActor::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<AArrowActor>();
		Mngr.AddConstructor<AArrowActor>();
		Mngr.AddBases<AArrowActor, AActor>();
;
        AArrowActor::StaticClass_->Type = Type_of<AArrowActor>;
        AArrowActor::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<AArrowActor>);
    }

    static TClassRegistry<AArrowActor> Dummy;
};
TClassRegistry<AArrowActor> Dummy = TClassRegistry<AArrowActor>("AArrowActor");


