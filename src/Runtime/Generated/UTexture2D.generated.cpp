#include "D:/Nilou/src/Runtime/Rendering/Texture2D.h"
namespace nilou {
std::string UTexture2D::GetClassName() { return "UTexture2D"; }
EUClasses UTexture2D::GetClassEnum() { return EUClasses::MC_UTexture2D; }
const UClass *UTexture2D::GetClass() { return UTexture2D::StaticClass(); }
const UClass *UTexture2D::StaticClass()
{
	static UClass *StaticClass = new UClass("UTexture2D", EUClasses::MC_UTexture2D);
	return StaticClass;
}
std::unique_ptr<UObject> UTexture2D::CreateDefaultObject()
{
    return std::make_unique<UTexture2D>();
}
}
