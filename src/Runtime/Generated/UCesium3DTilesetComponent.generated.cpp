#include "../../src/Runtime/Framework/Common/Components/Cesium3DTilesetComponent.h"
namespace nilou {
std::string UCesium3DTilesetComponent::GetClassName() { return "UCesium3DTilesetComponent"; }
EUClasses UCesium3DTilesetComponent::GetClassEnum() { return EUClasses::MC_UCesium3DTilesetComponent; }
const UClass *UCesium3DTilesetComponent::GetClass() { return UCesium3DTilesetComponent::StaticClass(); }
const UClass *UCesium3DTilesetComponent::StaticClass()
{
	static UClass *StaticClass = new UClass("UCesium3DTilesetComponent", EUClasses::MC_UCesium3DTilesetComponent);
	return StaticClass;
}
}
