#include "../../src/Runtime/Framework/Common/Actor/ArrowActor.h"
namespace nilou {
std::string AArrowActor::GetClassName() { return "AArrowActor"; }
EUClasses AArrowActor::GetClassEnum() { return EUClasses::MC_AArrowActor; }
const UClass *AArrowActor::GetClass() { return AArrowActor::StaticClass(); }
const UClass *AArrowActor::StaticClass()
{
	static UClass *StaticClass = new UClass("AArrowActor", EUClasses::MC_AArrowActor);
	return StaticClass;
}
}
