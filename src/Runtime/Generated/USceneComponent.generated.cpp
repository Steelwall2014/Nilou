#include "../../src/Runtime/Framework/Common/Components/SceneComponent.h"
namespace nilou {
std::string USceneComponent::GetClassName() { return "USceneComponent"; }
EUClasses USceneComponent::GetClassEnum() { return EUClasses::MC_USceneComponent; }
const UClass *USceneComponent::GetClass() { return USceneComponent::StaticClass(); }
const UClass *USceneComponent::StaticClass()
{
	static UClass *StaticClass = new UClass("USceneComponent", EUClasses::MC_USceneComponent);
	return StaticClass;
}
}
