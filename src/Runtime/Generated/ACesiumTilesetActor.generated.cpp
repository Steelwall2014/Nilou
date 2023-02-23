#include "D:/Nilou/src/Runtime/Framework/Common/Actor/Cesium3DTilesetActor.h"
namespace nilou {
std::string ACesiumTilesetActor::GetClassName() { return "ACesiumTilesetActor"; }
EUClasses ACesiumTilesetActor::GetClassEnum() { return EUClasses::MC_ACesiumTilesetActor; }
const UClass *ACesiumTilesetActor::GetClass() { return ACesiumTilesetActor::StaticClass(); }
const UClass *ACesiumTilesetActor::StaticClass()
{
	static UClass *StaticClass = new UClass("ACesiumTilesetActor", EUClasses::MC_ACesiumTilesetActor);
	return StaticClass;
}
std::unique_ptr<UObject> ACesiumTilesetActor::CreateDefaultObject()
{
    return std::make_unique<ACesiumTilesetActor>();
}
}
