#include "D:/Nilou/src/Runtime/Framework/Common/Components/PrimitiveComponent.h"
namespace nilou {
std::string UPrimitiveComponent::GetClassName() { return "UPrimitiveComponent"; }
EUClasses UPrimitiveComponent::GetClassEnum() { return EUClasses::MC_UPrimitiveComponent; }
const UClass *UPrimitiveComponent::GetClass() { return UPrimitiveComponent::StaticClass(); }
const UClass *UPrimitiveComponent::StaticClass()
{
	static UClass *StaticClass = new UClass("UPrimitiveComponent", EUClasses::MC_UPrimitiveComponent);
	return StaticClass;
}
std::unique_ptr<UObject> UPrimitiveComponent::CreateDefaultObject()
{
    return std::make_unique<UPrimitiveComponent>();
}
}
