#include "../../src/Runtime/Framework/Common/Components/MeshComponent.h"
namespace nilou {
std::string UStaticMeshComponent::GetClassName() { return "UStaticMeshComponent"; }
EUClasses UStaticMeshComponent::GetClassEnum() { return EUClasses::MC_UStaticMeshComponent; }
const UClass *UStaticMeshComponent::GetClass() { return UStaticMeshComponent::StaticClass(); }
const UClass *UStaticMeshComponent::StaticClass()
{
	static UClass *StaticClass = new UClass("UStaticMeshComponent", EUClasses::MC_UStaticMeshComponent);
	return StaticClass;
}
}
