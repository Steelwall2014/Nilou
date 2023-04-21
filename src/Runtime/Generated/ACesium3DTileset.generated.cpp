#include "D:/Nilou/src/Runtime/Cesium3DTiles/Cesium3DTileset.h"
namespace nilou {
std::string ACesium3DTileset::GetClassName() { return "ACesium3DTileset"; }
EUClasses ACesium3DTileset::GetClassEnum() { return EUClasses::MC_ACesium3DTileset; }
const UClass *ACesium3DTileset::GetClass() { return ACesium3DTileset::StaticClass(); }
const UClass *ACesium3DTileset::StaticClass()
{
	static UClass *StaticClass = new UClass("ACesium3DTileset", EUClasses::MC_ACesium3DTileset);
	return StaticClass;
}
std::unique_ptr<UObject> ACesium3DTileset::CreateDefaultObject()
{
    return std::make_unique<ACesium3DTileset>();
}
}
