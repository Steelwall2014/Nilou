#include "../../src/Runtime/Framework/Common/Components/LightComponent.h"
namespace nilou {
std::string ULightComponent::GetClassName() { return "ULightComponent"; }
EUClasses ULightComponent::GetClassEnum() { return EUClasses::MC_ULightComponent; }
const UClass *ULightComponent::GetClass() { return ULightComponent::StaticClass(); }
const UClass *ULightComponent::StaticClass()
{
	static UClass *StaticClass = new UClass("ULightComponent", EUClasses::MC_ULightComponent);
	return StaticClass;
}
}