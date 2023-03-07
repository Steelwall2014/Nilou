#include "D:/Nilou/src/Runtime/Rendering/Texture.h"
namespace nilou {
std::string UVirtualTexture::GetClassName() { return "UVirtualTexture"; }
EUClasses UVirtualTexture::GetClassEnum() { return EUClasses::MC_UVirtualTexture; }
const UClass *UVirtualTexture::GetClass() { return UVirtualTexture::StaticClass(); }
const UClass *UVirtualTexture::StaticClass()
{
	static UClass *StaticClass = new UClass("UVirtualTexture", EUClasses::MC_UVirtualTexture);
	return StaticClass;
}
std::unique_ptr<UObject> UVirtualTexture::CreateDefaultObject()
{
    return std::make_unique<UVirtualTexture>();
}
}
