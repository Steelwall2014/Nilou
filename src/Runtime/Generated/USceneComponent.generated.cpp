#include "D:/Nilou/src/Runtime/Cesium3DTiles/Cesium3DTileComponent.h"
#include <UDRefl/UDRefl.hpp>

using namespace nilou;
using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> USceneComponent::StaticClass_ = nullptr;
const NClass *USceneComponent::GetClass() const 
{ 
    return USceneComponent::StaticClass(); 
}
const NClass *USceneComponent::StaticClass()
{
    return USceneComponent::StaticClass_.get();
}

template<>
struct TClassRegistry<USceneComponent>
{
    TClassRegistry(const std::string& InName)
    {
        USceneComponent::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<USceneComponent>();
		Mngr.AddConstructor<USceneComponent>();
		Mngr.AddBases<USceneComponent, UActorComponent>();
;
        USceneComponent::StaticClass_->Type = Type_of<USceneComponent>;
        USceneComponent::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<USceneComponent>);
    }

    static TClassRegistry<USceneComponent> Dummy;
};
TClassRegistry<USceneComponent> Dummy = TClassRegistry<USceneComponent>("USceneComponent");


