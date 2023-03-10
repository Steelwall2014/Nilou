#include "D:/Nilou/src/Runtime/Framework/Common/Actor/FFTOceanActor.h"
namespace nilou {
std::string AFFTOceanActor::GetClassName() { return "AFFTOceanActor"; }
EUClasses AFFTOceanActor::GetClassEnum() { return EUClasses::MC_AFFTOceanActor; }
const UClass *AFFTOceanActor::GetClass() { return AFFTOceanActor::StaticClass(); }
const UClass *AFFTOceanActor::StaticClass()
{
	static UClass *StaticClass = new UClass("AFFTOceanActor", EUClasses::MC_AFFTOceanActor);
	return StaticClass;
}
std::unique_ptr<UObject> AFFTOceanActor::CreateDefaultObject()
{
    return std::make_unique<AFFTOceanActor>();
}
}
