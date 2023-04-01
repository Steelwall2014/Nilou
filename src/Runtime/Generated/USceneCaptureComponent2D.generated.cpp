#include "D:/Nilou/src/Runtime/Framework/Common/Components/SceneCaptureComponent.h"
namespace nilou {
std::string USceneCaptureComponent2D::GetClassName() { return "USceneCaptureComponent2D"; }
EUClasses USceneCaptureComponent2D::GetClassEnum() { return EUClasses::MC_USceneCaptureComponent2D; }
const UClass *USceneCaptureComponent2D::GetClass() { return USceneCaptureComponent2D::StaticClass(); }
const UClass *USceneCaptureComponent2D::StaticClass()
{
	static UClass *StaticClass = new UClass("USceneCaptureComponent2D", EUClasses::MC_USceneCaptureComponent2D);
	return StaticClass;
}
std::unique_ptr<UObject> USceneCaptureComponent2D::CreateDefaultObject()
{
    return std::make_unique<USceneCaptureComponent2D>();
}
}
