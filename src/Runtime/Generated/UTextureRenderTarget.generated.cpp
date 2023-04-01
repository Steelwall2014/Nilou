#include "D:/Nilou/src/Runtime/Rendering/Texture.h"
namespace nilou {
std::string UTextureRenderTarget::GetClassName() { return "UTextureRenderTarget"; }
EUClasses UTextureRenderTarget::GetClassEnum() { return EUClasses::MC_UTextureRenderTarget; }
const UClass *UTextureRenderTarget::GetClass() { return UTextureRenderTarget::StaticClass(); }
const UClass *UTextureRenderTarget::StaticClass()
{
	static UClass *StaticClass = new UClass("UTextureRenderTarget", EUClasses::MC_UTextureRenderTarget);
	return StaticClass;
}
std::unique_ptr<UObject> UTextureRenderTarget::CreateDefaultObject()
{
    return std::make_unique<UTextureRenderTarget>();
}
}
