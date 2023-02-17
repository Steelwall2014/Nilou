#include "D:/Nilou/src/Runtime/Framework/Common/Components/LineBatchComponent.h"
namespace nilou {
std::string ULineBatchComponent::GetClassName() { return "ULineBatchComponent"; }
EUClasses ULineBatchComponent::GetClassEnum() { return EUClasses::MC_ULineBatchComponent; }
const UClass *ULineBatchComponent::GetClass() { return ULineBatchComponent::StaticClass(); }
const UClass *ULineBatchComponent::StaticClass()
{
	static UClass *StaticClass = new UClass("ULineBatchComponent", EUClasses::MC_ULineBatchComponent);
	return StaticClass;
}
}
