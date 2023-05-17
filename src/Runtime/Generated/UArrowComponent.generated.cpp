#include "D:/Nilou/src/Runtime/Framework/Common/Actor/ArrowActor.h"
#include <UDRefl/UDRefl.hpp>

using namespace nilou;
using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> UArrowComponent::StaticClass_ = nullptr;
const NClass *UArrowComponent::GetClass() const 
{ 
    return UArrowComponent::StaticClass(); 
}
const NClass *UArrowComponent::StaticClass()
{
    return UArrowComponent::StaticClass_.get();
}

template<>
struct TClassRegistry<UArrowComponent>
{
    TClassRegistry(const std::string& InName)
    {
        UArrowComponent::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<UArrowComponent>();
		Mngr.AddConstructor<UArrowComponent>();
		Mngr.AddBases<UArrowComponent, UPrimitiveComponent>();
;
        UArrowComponent::StaticClass_->Type = Type_of<UArrowComponent>;
        UArrowComponent::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<UArrowComponent>);
    }

    static TClassRegistry<UArrowComponent> Dummy;
};
TClassRegistry<UArrowComponent> Dummy = TClassRegistry<UArrowComponent>("UArrowComponent");


