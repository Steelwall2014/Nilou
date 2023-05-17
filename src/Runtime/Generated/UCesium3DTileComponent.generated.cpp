#include "D:/Nilou/src/Runtime/Cesium3DTiles/Cesium3DTileComponent.h"
#include <UDRefl/UDRefl.hpp>

using namespace nilou;
using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> UCesium3DTileComponent::StaticClass_ = nullptr;
const NClass *UCesium3DTileComponent::GetClass() const 
{ 
    return UCesium3DTileComponent::StaticClass(); 
}
const NClass *UCesium3DTileComponent::StaticClass()
{
    return UCesium3DTileComponent::StaticClass_.get();
}

template<>
struct TClassRegistry<UCesium3DTileComponent>
{
    TClassRegistry(const std::string& InName)
    {
        UCesium3DTileComponent::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<UCesium3DTileComponent>();
		Mngr.AddConstructor<UCesium3DTileComponent>();
		Mngr.AddBases<UCesium3DTileComponent, UPrimitiveComponent>();
;
        UCesium3DTileComponent::StaticClass_->Type = Type_of<UCesium3DTileComponent>;
        UCesium3DTileComponent::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<UCesium3DTileComponent>);
    }

    static TClassRegistry<UCesium3DTileComponent> Dummy;
};
TClassRegistry<UCesium3DTileComponent> Dummy = TClassRegistry<UCesium3DTileComponent>("UCesium3DTileComponent");


