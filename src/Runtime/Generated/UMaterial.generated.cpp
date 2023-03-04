#include "../../src/Runtime/Rendering/Material.h"
namespace nilou {
std::string UMaterial::GetClassName() { return "UMaterial"; }
EUClasses UMaterial::GetClassEnum() { return EUClasses::MC_UMaterial; }
const UClass *UMaterial::GetClass() { return UMaterial::StaticClass(); }
const UClass *UMaterial::StaticClass()
{
	static UClass *StaticClass = new UClass("UMaterial", EUClasses::MC_UMaterial);
	return StaticClass;
}
std::unique_ptr<UObject> UMaterial::CreateDefaultObject()
{
    return std::make_unique<UMaterial>();
}
}
