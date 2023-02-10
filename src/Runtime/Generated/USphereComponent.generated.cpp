#include "../../src/Runtime/Framework/Common/Components/SphereComponent.h"
namespace nilou {
std::string USphereComponent::GetClassName() { return "USphereComponent"; }
EUClasses USphereComponent::GetClassEnum() { return EUClasses::MC_USphereComponent; }
const UClass *USphereComponent::GetClass() { return USphereComponent::StaticClass(); }
const UClass *USphereComponent::StaticClass()
{
	static UClass *StaticClass = new UClass("USphereComponent", EUClasses::MC_USphereComponent);
	return StaticClass;
}
}
