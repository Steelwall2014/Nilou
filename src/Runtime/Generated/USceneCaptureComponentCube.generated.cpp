#include "D:/Nilou/src/Runtime/Framework/Common/Components/SceneCaptureComponent.h"
namespace nilou {
std::string USceneCaptureComponentCube::GetClassName() { return "USceneCaptureComponentCube"; }
EUClasses USceneCaptureComponentCube::GetClassEnum() { return EUClasses::MC_USceneCaptureComponentCube; }
const UClass *USceneCaptureComponentCube::GetClass() { return USceneCaptureComponentCube::StaticClass(); }
const UClass *USceneCaptureComponentCube::StaticClass()
{
	static UClass *StaticClass = new UClass("USceneCaptureComponentCube", EUClasses::MC_USceneCaptureComponentCube);
	return StaticClass;
}
std::unique_ptr<UObject> USceneCaptureComponentCube::CreateDefaultObject()
{
    return std::make_unique<USceneCaptureComponentCube>();
}
}
