#include "D:/Nilou/src/Runtime/Framework/Common/Actor/SphereActor.h"
#include <UDRefl/UDRefl.hpp>

using namespace nilou;
using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> ASphereActor::StaticClass_ = nullptr;
const NClass *ASphereActor::GetClass() const 
{ 
    return ASphereActor::StaticClass(); 
}
const NClass *ASphereActor::StaticClass()
{
    return ASphereActor::StaticClass_.get();
}

template<>
struct TClassRegistry<ASphereActor>
{
    TClassRegistry(const std::string& InName)
    {
        ASphereActor::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<ASphereActor>();
		Mngr.AddConstructor<ASphereActor>();
		Mngr.AddBases<ASphereActor, AActor>();
;
        ASphereActor::StaticClass_->Type = Type_of<ASphereActor>;
        ASphereActor::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<ASphereActor>);
    }

    static TClassRegistry<ASphereActor> Dummy;
};
TClassRegistry<ASphereActor> Dummy = TClassRegistry<ASphereActor>("ASphereActor");


