#include "../../src/Runtime/Framework/Common/Actor/LineBatchActor.h"
namespace nilou {
std::string ALineBatchActor::GetClassName() { return "ALineBatchActor"; }
EUClasses ALineBatchActor::GetClassEnum() { return EUClasses::MC_ALineBatchActor; }
const UClass *ALineBatchActor::GetClass() { return ALineBatchActor::StaticClass(); }
const UClass *ALineBatchActor::StaticClass()
{
	static UClass *StaticClass = new UClass("ALineBatchActor", EUClasses::MC_ALineBatchActor);
	return StaticClass;
}
std::unique_ptr<UObject> ALineBatchActor::CreateDefaultObject()
{
    return std::make_unique<ALineBatchActor>();
}
}
