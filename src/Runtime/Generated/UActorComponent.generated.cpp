#include "../../src/Runtime/Framework/Common/Components/ActorComponent.h"
namespace nilou {
std::string UActorComponent::GetClassName() { return "UActorComponent"; }
EUClasses UActorComponent::GetClassEnum() { return EUClasses::MC_UActorComponent; }
const UClass *UActorComponent::GetClass() { return UActorComponent::StaticClass(); }
const UClass *UActorComponent::StaticClass()
{
	static UClass *StaticClass = new UClass("UActorComponent", EUClasses::MC_UActorComponent);
	return StaticClass;
}
}
