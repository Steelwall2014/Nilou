#include "D:/Nilou/src/Runtime/Framework/Common/CoreUObject/Object.h"
namespace nilou {
std::string UObject::GetClassName() { return "UObject"; }
EUClasses UObject::GetClassEnum() { return EUClasses::MC_UObject; }
const UClass *UObject::GetClass() { return UObject::StaticClass(); }
const UClass *UObject::StaticClass()
{
	static UClass *StaticClass = new UClass("UObject", EUClasses::MC_UObject);
	return StaticClass;
}
std::unique_ptr<UObject> UObject::CreateDefaultObject()
{
    return std::make_unique<UObject>();
}
}
