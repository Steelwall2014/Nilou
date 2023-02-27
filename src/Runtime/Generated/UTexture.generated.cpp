#include "../../src/Runtime/Rendering/Texture.h"
namespace nilou {
std::string UTexture::GetClassName() { return "UTexture"; }
EUClasses UTexture::GetClassEnum() { return EUClasses::MC_UTexture; }
const UClass *UTexture::GetClass() { return UTexture::StaticClass(); }
const UClass *UTexture::StaticClass()
{
	static UClass *StaticClass = new UClass("UTexture", EUClasses::MC_UTexture);
	return StaticClass;
}
std::unique_ptr<UObject> UTexture::CreateDefaultObject()
{
    return std::make_unique<UTexture>();
}
}
