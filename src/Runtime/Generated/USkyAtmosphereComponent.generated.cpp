#include "../../src/Runtime/Framework/Common/Components/SkyAtmosphereComponent.h"
namespace nilou {
std::string USkyAtmosphereComponent::GetClassName() { return "USkyAtmosphereComponent"; }
EUClasses USkyAtmosphereComponent::GetClassEnum() { return EUClasses::MC_USkyAtmosphereComponent; }
const UClass *USkyAtmosphereComponent::GetClass() { return USkyAtmosphereComponent::StaticClass(); }
const UClass *USkyAtmosphereComponent::StaticClass()
{
	static UClass *StaticClass = new UClass("USkyAtmosphereComponent", EUClasses::MC_USkyAtmosphereComponent);
	return StaticClass;
}
std::unique_ptr<UObject> USkyAtmosphereComponent::CreateDefaultObject()
{
    return std::make_unique<USkyAtmosphereComponent>();
}
}
