#include "D:/Nilou/src/Runtime/Rendering/Texture2DArray.h"
namespace nilou {
std::string UTexture2DArray::GetClassName() { return "UTexture2DArray"; }
EUClasses UTexture2DArray::GetClassEnum() { return EUClasses::MC_UTexture2DArray; }
const UClass *UTexture2DArray::GetClass() { return UTexture2DArray::StaticClass(); }
const UClass *UTexture2DArray::StaticClass()
{
	static UClass *StaticClass = new UClass("UTexture2DArray", EUClasses::MC_UTexture2DArray);
	return StaticClass;
}
std::unique_ptr<UObject> UTexture2DArray::CreateDefaultObject()
{
    return std::make_unique<UTexture2DArray>();
}
}
