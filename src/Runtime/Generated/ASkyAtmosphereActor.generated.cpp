#include "D:/Nilou/src/Runtime/Framework/Common/Actor/SkyAtmosphereActor.h"
#include <UDRefl/UDRefl.hpp>

using namespace nilou;
using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> ASkyAtmosphereActor::StaticClass_ = nullptr;
const NClass *ASkyAtmosphereActor::GetClass() const 
{ 
    return ASkyAtmosphereActor::StaticClass(); 
}
const NClass *ASkyAtmosphereActor::StaticClass()
{
    return ASkyAtmosphereActor::StaticClass_.get();
}

template<>
struct TClassRegistry<ASkyAtmosphereActor>
{
    TClassRegistry(const std::string& InName)
    {
        ASkyAtmosphereActor::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<ASkyAtmosphereActor>();
		Mngr.AddConstructor<ASkyAtmosphereActor>();
		Mngr.AddBases<ASkyAtmosphereActor, AActor>();
;
        ASkyAtmosphereActor::StaticClass_->Type = Type_of<ASkyAtmosphereActor>;
        ASkyAtmosphereActor::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<ASkyAtmosphereActor>);
    }

    static TClassRegistry<ASkyAtmosphereActor> Dummy;
};
TClassRegistry<ASkyAtmosphereActor> Dummy = TClassRegistry<ASkyAtmosphereActor>("ASkyAtmosphereActor");


