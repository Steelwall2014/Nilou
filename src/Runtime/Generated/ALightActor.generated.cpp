#include "D:/Nilou/src/Runtime/Framework/Common/Actor/LightActor.h"
namespace nilou {
std::string ALightActor::GetClassName() { return "ALightActor"; }
EUClasses ALightActor::GetClassEnum() { return EUClasses::MC_ALightActor; }
const UClass *ALightActor::GetClass() { return ALightActor::StaticClass(); }
const UClass *ALightActor::StaticClass()
{
	static UClass *StaticClass = new UClass("ALightActor", EUClasses::MC_ALightActor);
	return StaticClass;
}
}
