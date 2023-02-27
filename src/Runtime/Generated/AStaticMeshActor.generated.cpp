#include "../../src/Runtime/Framework/Common/Actor/StaticMeshActor.h"
namespace nilou {
std::string AStaticMeshActor::GetClassName() { return "AStaticMeshActor"; }
EUClasses AStaticMeshActor::GetClassEnum() { return EUClasses::MC_AStaticMeshActor; }
const UClass *AStaticMeshActor::GetClass() { return AStaticMeshActor::StaticClass(); }
const UClass *AStaticMeshActor::StaticClass()
{
	static UClass *StaticClass = new UClass("AStaticMeshActor", EUClasses::MC_AStaticMeshActor);
	return StaticClass;
}
std::unique_ptr<UObject> AStaticMeshActor::CreateDefaultObject()
{
    return std::make_unique<AStaticMeshActor>();
}
}
