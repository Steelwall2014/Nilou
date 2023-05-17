#include "D:/Nilou/src/Runtime/Framework/Common/Actor/FFTOceanActor.h"
#include <UDRefl/UDRefl.hpp>

using namespace nilou;
using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> UMaterialInstance::StaticClass_ = nullptr;
const NClass *UMaterialInstance::GetClass() const 
{ 
    return UMaterialInstance::StaticClass(); 
}
const NClass *UMaterialInstance::StaticClass()
{
    return UMaterialInstance::StaticClass_.get();
}

template<>
struct TClassRegistry<UMaterialInstance>
{
    TClassRegistry(const std::string& InName)
    {
        UMaterialInstance::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<UMaterialInstance>();
		Mngr.AddConstructor<UMaterialInstance>();
		Mngr.AddConstructor<UMaterialInstance, nilou::UMaterial *>();
		Mngr.AddBases<UMaterialInstance, UMaterial>();
;
        UMaterialInstance::StaticClass_->Type = Type_of<UMaterialInstance>;
        UMaterialInstance::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<UMaterialInstance>);
    }

    static TClassRegistry<UMaterialInstance> Dummy;
};
TClassRegistry<UMaterialInstance> Dummy = TClassRegistry<UMaterialInstance>("UMaterialInstance");


