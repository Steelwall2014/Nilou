#include "D:/Nilou/src/Runtime/Framework/Common/Actor/ReflectionProbe.h"
namespace nilou {
std::string AReflectionProbe::GetClassName() { return "AReflectionProbe"; }
EUClasses AReflectionProbe::GetClassEnum() { return EUClasses::MC_AReflectionProbe; }
const UClass *AReflectionProbe::GetClass() { return AReflectionProbe::StaticClass(); }
const UClass *AReflectionProbe::StaticClass()
{
	static UClass *StaticClass = new UClass("AReflectionProbe", EUClasses::MC_AReflectionProbe);
	return StaticClass;
}
std::unique_ptr<UObject> AReflectionProbe::CreateDefaultObject()
{
    return std::make_unique<AReflectionProbe>();
}
}
