#include "../../src/Runtime/Rendering/Material.h"
namespace nilou {
std::string UMaterialInstance::GetClassName() { return "UMaterialInstance"; }
EUClasses UMaterialInstance::GetClassEnum() { return EUClasses::MC_UMaterialInstance; }
const UClass *UMaterialInstance::GetClass() { return UMaterialInstance::StaticClass(); }
const UClass *UMaterialInstance::StaticClass()
{
	static UClass *StaticClass = new UClass("UMaterialInstance", EUClasses::MC_UMaterialInstance);
	return StaticClass;
}
}
