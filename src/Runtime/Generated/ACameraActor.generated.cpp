#include "../../src/Runtime/Framework/Common/Actor/CameraActor.h"
namespace nilou {
std::string ACameraActor::GetClassName() { return "ACameraActor"; }
EUClasses ACameraActor::GetClassEnum() { return EUClasses::MC_ACameraActor; }
const UClass *ACameraActor::GetClass() { return ACameraActor::StaticClass(); }
const UClass *ACameraActor::StaticClass()
{
	static UClass *StaticClass = new UClass("ACameraActor", EUClasses::MC_ACameraActor);
	return StaticClass;
}
}
