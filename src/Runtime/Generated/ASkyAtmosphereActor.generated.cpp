#include "D:/Nilou/src/Runtime/Framework/Common/Actor/SkyAtmosphereActor.h"
namespace nilou {
std::string ASkyAtmosphereActor::GetClassName() { return "ASkyAtmosphereActor"; }
EUClasses ASkyAtmosphereActor::GetClassEnum() { return EUClasses::MC_ASkyAtmosphereActor; }
const UClass *ASkyAtmosphereActor::GetClass() { return ASkyAtmosphereActor::StaticClass(); }
const UClass *ASkyAtmosphereActor::StaticClass()
{
	static UClass *StaticClass = new UClass("ASkyAtmosphereActor", EUClasses::MC_ASkyAtmosphereActor);
	return StaticClass;
}
std::unique_ptr<UObject> ASkyAtmosphereActor::CreateDefaultObject()
{
    return std::make_unique<ASkyAtmosphereActor>();
}
}
