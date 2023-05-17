#include "D:/Nilou/src/Runtime/Cesium3DTiles/Cesium3DTileComponent.h"
#include <UDRefl/UDRefl.hpp>

using namespace nilou;
using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> UPrimitiveComponent::StaticClass_ = nullptr;
const NClass *UPrimitiveComponent::GetClass() const 
{ 
    return UPrimitiveComponent::StaticClass(); 
}
const NClass *UPrimitiveComponent::StaticClass()
{
    return UPrimitiveComponent::StaticClass_.get();
}

template<>
struct TClassRegistry<UPrimitiveComponent>
{
    TClassRegistry(const std::string& InName)
    {
        UPrimitiveComponent::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<UPrimitiveComponent>();
		Mngr.AddConstructor<UPrimitiveComponent>();
		Mngr.AddBases<UPrimitiveComponent, USceneComponent>();
;
        UPrimitiveComponent::StaticClass_->Type = Type_of<UPrimitiveComponent>;
        UPrimitiveComponent::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<UPrimitiveComponent>);
    }

    static TClassRegistry<UPrimitiveComponent> Dummy;
};
TClassRegistry<UPrimitiveComponent> Dummy = TClassRegistry<UPrimitiveComponent>("UPrimitiveComponent");


