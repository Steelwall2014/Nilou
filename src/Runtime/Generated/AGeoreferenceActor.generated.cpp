#include "../../src/Runtime/Framework/Common/Actor/GeoreferenceActor.h"
namespace nilou {
std::string AGeoreferenceActor::GetClassName() { return "AGeoreferenceActor"; }
EUClasses AGeoreferenceActor::GetClassEnum() { return EUClasses::MC_AGeoreferenceActor; }
const UClass *AGeoreferenceActor::GetClass() { return AGeoreferenceActor::StaticClass(); }
const UClass *AGeoreferenceActor::StaticClass()
{
	static UClass *StaticClass = new UClass("AGeoreferenceActor", EUClasses::MC_AGeoreferenceActor);
	return StaticClass;
}
}
