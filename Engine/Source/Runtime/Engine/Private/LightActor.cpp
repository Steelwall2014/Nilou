#include "GameFramework/LightActor.h"
#include "InputManager.h"
#include "Math/Maths.h"

namespace nilou {

    ALightActor::ALightActor()
    {
        LightComponent = CreateComponent<ULightComponent>(this, "LightComponent");
        LightComponent->AttachToComponent(GetRootComponent());

        InputAxisMapping SunYaw_mapping("SunYaw");
        SunYaw_mapping.AddGroup(InputKey::KEY_RIGHT, 1.0f);
        SunYaw_mapping.AddGroup(InputKey::KEY_LEFT, -1.0f);
        GetInputManager()->BindAxis(SunYaw_mapping, this, &ALightActor::RotateSunYaw);

        InputAxisMapping SunPitch_mapping("SunPitch");
        SunPitch_mapping.AddGroup(InputKey::KEY_UP, 1.0f);
        SunPitch_mapping.AddGroup(InputKey::KEY_DOWN, -1.0f);
        GetInputManager()->BindAxis(SunPitch_mapping, this, &ALightActor::RotateSunPitch);
    }

    void ALightActor::Tick(double DeltaTime)
    {
        AActor::Tick(DeltaTime);

        if (SunYawInput == 0.f && SunPitchInput == 0.f)
            return;

        FRotator NewRotation = GetActorRotator();
        const float DeltaYaw = SunYawInput * SunRotationSpeed * static_cast<float>(DeltaTime);
        const float DeltaPitch = -SunPitchInput * SunRotationSpeed * static_cast<float>(DeltaTime);
        NewRotation.Yaw += DeltaYaw;
        NewRotation.Pitch = glm::clamp(
            static_cast<float>(NewRotation.Pitch) + DeltaPitch,
            -89.0f,
            89.0f);
        SetActorRotator(NewRotation);

        SunYawInput = 0.f;
        SunPitchInput = 0.f;
    }

    void ALightActor::RotateSunYaw(float AxisValue)
    {
        SunYawInput = AxisValue;
    }

    void ALightActor::RotateSunPitch(float AxisValue)
    {
        SunPitchInput = AxisValue;
    }
}
