#include "D:/Nilou/src/Runtime/Cesium3DTiles/Cesium3DTileComponent.h"
#include <UDRefl/UDRefl.hpp>

using namespace nilou;
using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> UActorComponent::StaticClass_ = nullptr;
const NClass *UActorComponent::GetClass() const 
{ 
    return UActorComponent::StaticClass(); 
}
const NClass *UActorComponent::StaticClass()
{
    return UActorComponent::StaticClass_.get();
}

template<>
struct TClassRegistry<UActorComponent>
{
    TClassRegistry(const std::string& InName)
    {
        UActorComponent::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<UActorComponent>();
		Mngr.AddConstructor<UActorComponent>();
		Mngr.AddBases<UActorComponent, UObject>();
;
        UActorComponent::StaticClass_->Type = Type_of<UActorComponent>;
        UActorComponent::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<UActorComponent>);
    }

    static TClassRegistry<UActorComponent> Dummy;
};
TClassRegistry<UActorComponent> Dummy = TClassRegistry<UActorComponent>("UActorComponent");


