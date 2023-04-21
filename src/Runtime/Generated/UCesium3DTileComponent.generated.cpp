#include "D:/Nilou/src/Runtime/Cesium3DTiles/Cesium3DTileComponent.h"
namespace nilou {
std::string UCesium3DTileComponent::GetClassName() { return "UCesium3DTileComponent"; }
EUClasses UCesium3DTileComponent::GetClassEnum() { return EUClasses::MC_UCesium3DTileComponent; }
const UClass *UCesium3DTileComponent::GetClass() { return UCesium3DTileComponent::StaticClass(); }
const UClass *UCesium3DTileComponent::StaticClass()
{
	static UClass *StaticClass = new UClass("UCesium3DTileComponent", EUClasses::MC_UCesium3DTileComponent);
	return StaticClass;
}
std::unique_ptr<UObject> UCesium3DTileComponent::CreateDefaultObject()
{
    return std::make_unique<UCesium3DTileComponent>();
}
}
