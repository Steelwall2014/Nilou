#include "D:/Nilou/src/Runtime/Framework/Common/Actor/Actor.h"
namespace nilou {
std::string AActor::GetClassName() { return "AActor"; }
EUClasses AActor::GetClassEnum() { return EUClasses::MC_AActor; }
const UClass *AActor::GetClass() { return AActor::StaticClass(); }
const UClass *AActor::StaticClass()
{
	static UClass *StaticClass = new UClass("AActor", EUClasses::MC_AActor);
	return StaticClass;
}
std::unique_ptr<UObject> AActor::CreateDefaultObject()
{
    return std::make_unique<AActor>();
}
}
