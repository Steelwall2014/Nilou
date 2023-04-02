#include "D:/Nilou/src/Runtime/Rendering/Texture3D.h"
namespace nilou {
std::string UTexture3D::GetClassName() { return "UTexture3D"; }
EUClasses UTexture3D::GetClassEnum() { return EUClasses::MC_UTexture3D; }
const UClass *UTexture3D::GetClass() { return UTexture3D::StaticClass(); }
const UClass *UTexture3D::StaticClass()
{
	static UClass *StaticClass = new UClass("UTexture3D", EUClasses::MC_UTexture3D);
	return StaticClass;
}
std::unique_ptr<UObject> UTexture3D::CreateDefaultObject()
{
    return std::make_unique<UTexture3D>();
}
}
