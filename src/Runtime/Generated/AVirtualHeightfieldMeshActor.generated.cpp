#include "D:/Nilou/src/Runtime/Framework/Common/Actor/VirtualHeightfieldMeshActor.h"
namespace nilou {
std::string AVirtualHeightfieldMeshActor::GetClassName() { return "AVirtualHeightfieldMeshActor"; }
EUClasses AVirtualHeightfieldMeshActor::GetClassEnum() { return EUClasses::MC_AVirtualHeightfieldMeshActor; }
const UClass *AVirtualHeightfieldMeshActor::GetClass() { return AVirtualHeightfieldMeshActor::StaticClass(); }
const UClass *AVirtualHeightfieldMeshActor::StaticClass()
{
	static UClass *StaticClass = new UClass("AVirtualHeightfieldMeshActor", EUClasses::MC_AVirtualHeightfieldMeshActor);
	return StaticClass;
}
std::unique_ptr<UObject> AVirtualHeightfieldMeshActor::CreateDefaultObject()
{
    return std::make_unique<AVirtualHeightfieldMeshActor>();
}
}
