#include "D:/Nilou/src/Runtime/Rendering/Texture.h"
namespace nilou {
std::string UTextureRenderTarget2D::GetClassName() { return "UTextureRenderTarget2D"; }
EUClasses UTextureRenderTarget2D::GetClassEnum() { return EUClasses::MC_UTextureRenderTarget2D; }
const UClass *UTextureRenderTarget2D::GetClass() { return UTextureRenderTarget2D::StaticClass(); }
const UClass *UTextureRenderTarget2D::StaticClass()
{
	static UClass *StaticClass = new UClass("UTextureRenderTarget2D", EUClasses::MC_UTextureRenderTarget2D);
	return StaticClass;
}
std::unique_ptr<UObject> UTextureRenderTarget2D::CreateDefaultObject()
{
    return std::make_unique<UTextureRenderTarget2D>();
}
}
