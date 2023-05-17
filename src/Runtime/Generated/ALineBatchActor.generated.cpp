#include "D:/Nilou/src/Runtime/Framework/Common/Actor/LineBatchActor.h"
#include <UDRefl/UDRefl.hpp>

using namespace nilou;
using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> ALineBatchActor::StaticClass_ = nullptr;
const NClass *ALineBatchActor::GetClass() const 
{ 
    return ALineBatchActor::StaticClass(); 
}
const NClass *ALineBatchActor::StaticClass()
{
    return ALineBatchActor::StaticClass_.get();
}

template<>
struct TClassRegistry<ALineBatchActor>
{
    TClassRegistry(const std::string& InName)
    {
        ALineBatchActor::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<ALineBatchActor>();
		Mngr.AddConstructor<ALineBatchActor>();
		Mngr.AddBases<ALineBatchActor, AActor>();
;
        ALineBatchActor::StaticClass_->Type = Type_of<ALineBatchActor>;
        ALineBatchActor::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<ALineBatchActor>);
    }

    static TClassRegistry<ALineBatchActor> Dummy;
};
TClassRegistry<ALineBatchActor> Dummy = TClassRegistry<ALineBatchActor>("ALineBatchActor");


