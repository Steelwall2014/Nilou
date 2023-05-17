#include "D:/Nilou/src/Runtime/Framework/Common/Actor/LightActor.h"
#include <UDRefl/UDRefl.hpp>

using namespace nilou;
using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> ALightActor::StaticClass_ = nullptr;
const NClass *ALightActor::GetClass() const 
{ 
    return ALightActor::StaticClass(); 
}
const NClass *ALightActor::StaticClass()
{
    return ALightActor::StaticClass_.get();
}

template<>
struct TClassRegistry<ALightActor>
{
    TClassRegistry(const std::string& InName)
    {
        ALightActor::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<ALightActor>();
		Mngr.AddConstructor<ALightActor>();
		Mngr.AddBases<ALightActor, AActor>();
;
        ALightActor::StaticClass_->Type = Type_of<ALightActor>;
        ALightActor::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<ALightActor>);
    }

    static TClassRegistry<ALightActor> Dummy;
};
TClassRegistry<ALightActor> Dummy = TClassRegistry<ALightActor>("ALightActor");


