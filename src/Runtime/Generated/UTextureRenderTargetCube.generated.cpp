#include "D:/Nilou/src/Runtime/Rendering/Texture.h"
namespace nilou {
std::string UTextureRenderTargetCube::GetClassName() { return "UTextureRenderTargetCube"; }
EUClasses UTextureRenderTargetCube::GetClassEnum() { return EUClasses::MC_UTextureRenderTargetCube; }
const UClass *UTextureRenderTargetCube::GetClass() { return UTextureRenderTargetCube::StaticClass(); }
const UClass *UTextureRenderTargetCube::StaticClass()
{
	static UClass *StaticClass = new UClass("UTextureRenderTargetCube", EUClasses::MC_UTextureRenderTargetCube);
	return StaticClass;
}
std::unique_ptr<UObject> UTextureRenderTargetCube::CreateDefaultObject()
{
    return std::make_unique<UTextureRenderTargetCube>();
}
}
