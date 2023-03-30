#include "D:/Nilou/src/Runtime/Framework/Common/Components/SceneCaptureComponent.h"
namespace nilou {
std::string USceneCaptureComponent::GetClassName() { return "USceneCaptureComponent"; }
EUClasses USceneCaptureComponent::GetClassEnum() { return EUClasses::MC_USceneCaptureComponent; }
const UClass *USceneCaptureComponent::GetClass() { return USceneCaptureComponent::StaticClass(); }
const UClass *USceneCaptureComponent::StaticClass()
{
	static UClass *StaticClass = new UClass("USceneCaptureComponent", EUClasses::MC_USceneCaptureComponent);
	return StaticClass;
}
std::unique_ptr<UObject> USceneCaptureComponent::CreateDefaultObject()
{
    return std::make_unique<USceneCaptureComponent>();
}
}
