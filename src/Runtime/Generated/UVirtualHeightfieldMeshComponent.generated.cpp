#include "D:/Nilou/src/Runtime/Framework/Common/Components/VirtualHeightfieldMeshComponent.h"
namespace nilou {
std::string UVirtualHeightfieldMeshComponent::GetClassName() { return "UVirtualHeightfieldMeshComponent"; }
EUClasses UVirtualHeightfieldMeshComponent::GetClassEnum() { return EUClasses::MC_UVirtualHeightfieldMeshComponent; }
const UClass *UVirtualHeightfieldMeshComponent::GetClass() { return UVirtualHeightfieldMeshComponent::StaticClass(); }
const UClass *UVirtualHeightfieldMeshComponent::StaticClass()
{
	static UClass *StaticClass = new UClass("UVirtualHeightfieldMeshComponent", EUClasses::MC_UVirtualHeightfieldMeshComponent);
	return StaticClass;
}
}
