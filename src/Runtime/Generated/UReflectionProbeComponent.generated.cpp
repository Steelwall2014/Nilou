#include "D:/Nilou/src/Runtime/Framework/Common/Components/ReflectionProbeComponent.h"
namespace nilou {
std::string UReflectionProbeComponent::GetClassName() { return "UReflectionProbeComponent"; }
EUClasses UReflectionProbeComponent::GetClassEnum() { return EUClasses::MC_UReflectionProbeComponent; }
const UClass *UReflectionProbeComponent::GetClass() { return UReflectionProbeComponent::StaticClass(); }
const UClass *UReflectionProbeComponent::StaticClass()
{
	static UClass *StaticClass = new UClass("UReflectionProbeComponent", EUClasses::MC_UReflectionProbeComponent);
	return StaticClass;
}
std::unique_ptr<UObject> UReflectionProbeComponent::CreateDefaultObject()
{
    return std::make_unique<UReflectionProbeComponent>();
}
}
