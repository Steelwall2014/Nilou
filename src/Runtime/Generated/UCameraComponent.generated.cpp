#include "../../src/Runtime/Framework/Common/Components/CameraComponent.h"
namespace nilou {
std::string UCameraComponent::GetClassName() { return "UCameraComponent"; }
EUClasses UCameraComponent::GetClassEnum() { return EUClasses::MC_UCameraComponent; }
const UClass *UCameraComponent::GetClass() { return UCameraComponent::StaticClass(); }
const UClass *UCameraComponent::StaticClass()
{
	static UClass *StaticClass = new UClass("UCameraComponent", EUClasses::MC_UCameraComponent);
	return StaticClass;
}
std::unique_ptr<UObject> UCameraComponent::CreateDefaultObject()
{
    return std::make_unique<UCameraComponent>();
}
}
