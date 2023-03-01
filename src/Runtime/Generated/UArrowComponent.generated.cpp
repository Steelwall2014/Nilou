#include "D:/Nilou/src/Runtime/Framework/Common/Components/ArrowComponent.h"
namespace nilou {
std::string UArrowComponent::GetClassName() { return "UArrowComponent"; }
EUClasses UArrowComponent::GetClassEnum() { return EUClasses::MC_UArrowComponent; }
const UClass *UArrowComponent::GetClass() { return UArrowComponent::StaticClass(); }
const UClass *UArrowComponent::StaticClass()
{
	static UClass *StaticClass = new UClass("UArrowComponent", EUClasses::MC_UArrowComponent);
	return StaticClass;
}
std::unique_ptr<UObject> UArrowComponent::CreateDefaultObject()
{
    return std::make_unique<UArrowComponent>();
}
}
