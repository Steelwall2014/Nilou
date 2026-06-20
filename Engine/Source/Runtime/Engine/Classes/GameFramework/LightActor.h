#include "GameFramework/Actor.h"
#include "Components/LightComponent.h"

namespace nilou {

    class NCLASS ALightActor : public AActor
    {
		GENERATED_BODY()
    public:
        ALightActor();

        virtual void Tick(double DeltaTime) override;

        NPROPERTY()
        ULightComponent* LightComponent;

    protected:
        void RotateSunYaw(float AxisValue);
        void RotateSunPitch(float AxisValue);

        float SunYawInput = 0.f;
        float SunPitchInput = 0.f;
        float SunRotationSpeed = 60.0f;
    };

}