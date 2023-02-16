#include "D:/Nilou/src/Runtime/Framework/Common/Actor/SphereActor.h"
namespace nilou {
std::string ASphereActor::GetClassName() { return "ASphereActor"; }
EUClasses ASphereActor::GetClassEnum() { return EUClasses::MC_ASphereActor; }
const UClass *ASphereActor::GetClass() { return ASphereActor::StaticClass(); }
const UClass *ASphereActor::StaticClass()
{
	static UClass *StaticClass = new UClass("ASphereActor", EUClasses::MC_ASphereActor);
	return StaticClass;
}
}
