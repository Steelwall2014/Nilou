#include "D:/Nilou/src/Runtime/Framework/Common/Actor/FFTOceanActor.h"
#include <UDRefl/UDRefl.hpp>

using namespace nilou;
using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> UMaterial::StaticClass_ = nullptr;
const NClass *UMaterial::GetClass() const 
{ 
    return UMaterial::StaticClass(); 
}
const NClass *UMaterial::StaticClass()
{
    return UMaterial::StaticClass_.get();
}

template<>
struct TClassRegistry<UMaterial>
{
    TClassRegistry(const std::string& InName)
    {
        UMaterial::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<UMaterial>();
		Mngr.AddConstructor<UMaterial>();
		Mngr.AddConstructor<UMaterial, const nilou::UMaterial &>();
		Mngr.AddBases<UMaterial, UObject>();
;
        UMaterial::StaticClass_->Type = Type_of<UMaterial>;
        UMaterial::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<UMaterial>);
    }

    static TClassRegistry<UMaterial> Dummy;
};
TClassRegistry<UMaterial> Dummy = TClassRegistry<UMaterial>("UMaterial");


