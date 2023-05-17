#include "D:/Nilou/src/Runtime/Framework/Common/Actor/ReflectionProbe.h"
#include <UDRefl/UDRefl.hpp>

using namespace nilou;
using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> AReflectionProbe::StaticClass_ = nullptr;
const NClass *AReflectionProbe::GetClass() const 
{ 
    return AReflectionProbe::StaticClass(); 
}
const NClass *AReflectionProbe::StaticClass()
{
    return AReflectionProbe::StaticClass_.get();
}

template<>
struct TClassRegistry<AReflectionProbe>
{
    TClassRegistry(const std::string& InName)
    {
        AReflectionProbe::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<AReflectionProbe>();
		Mngr.AddConstructor<AReflectionProbe>();
		Mngr.AddBases<AReflectionProbe, AActor>();
;
        AReflectionProbe::StaticClass_->Type = Type_of<AReflectionProbe>;
        AReflectionProbe::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<AReflectionProbe>);
    }

    static TClassRegistry<AReflectionProbe> Dummy;
};
TClassRegistry<AReflectionProbe> Dummy = TClassRegistry<AReflectionProbe>("AReflectionProbe");


