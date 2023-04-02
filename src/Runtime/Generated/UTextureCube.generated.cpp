#include "D:/Nilou/src/Runtime/Rendering/TextureCube.h"
namespace nilou {
std::string UTextureCube::GetClassName() { return "UTextureCube"; }
EUClasses UTextureCube::GetClassEnum() { return EUClasses::MC_UTextureCube; }
const UClass *UTextureCube::GetClass() { return UTextureCube::StaticClass(); }
const UClass *UTextureCube::StaticClass()
{
	static UClass *StaticClass = new UClass("UTextureCube", EUClasses::MC_UTextureCube);
	return StaticClass;
}
std::unique_ptr<UObject> UTextureCube::CreateDefaultObject()
{
    return std::make_unique<UTextureCube>();
}
}
