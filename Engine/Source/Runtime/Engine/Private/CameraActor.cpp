#include "GameFramework/CameraActor.h"
#include "BaseApplication.h"
#include "InputManager.h"
#include "Math/Maths.h"
#include "Math/Transform.h"

namespace nilou {

    FQuat ROOT_ROTATION(1.f, 0.f, 0.f, 0.f);
    FVector ROOT_TRANSLATION(10, 10, 0);

    float MOVEMENTSPEED = 100;

    ACameraActor::ACameraActor()
    { 
        CameraComponent = CreateComponent<UCameraComponent>(this, "CameraComponent"); 
        CameraComponent->AttachToComponent(GetRootComponent());

        InputAxisMapping MoveForward_mapping("MoveForward");
        MoveForward_mapping.AddGroup(InputKey::KEY_W, 1.0f);
        MoveForward_mapping.AddGroup(InputKey::KEY_S, -1.0f);
        GetInputManager()->BindAxis(MoveForward_mapping, this, &ACameraActor::MoveForward);

        InputAxisMapping MoveRight_mapping("MoveRight");
        MoveRight_mapping.AddGroup(InputKey::KEY_D, 1.0f);
        MoveRight_mapping.AddGroup(InputKey::KEY_A, -1.0f);
        GetInputManager()->BindAxis(MoveRight_mapping, this, &ACameraActor::MoveRight);

        InputAxisMapping MoveUp_mapping("MoveUp");
        MoveUp_mapping.AddGroup(InputKey::KEY_E, 1.0f);
        MoveUp_mapping.AddGroup(InputKey::KEY_Q, -1.0f);
        GetInputManager()->BindAxis(MoveUp_mapping, this, &ACameraActor::MoveUp);

        InputAxisMapping YawCamera_mapping("YawCamera");
        YawCamera_mapping.AddGroup(InputKey::AXIS_MOUSEX, 1.0f);
        GetInputManager()->BindAxis(YawCamera_mapping, this, &ACameraActor::YawCamera);

        InputAxisMapping PitchCamera_mapping("PitchCamera");
        PitchCamera_mapping.AddGroup(InputKey::AXIS_MOUSEY, 1.0f);
        GetInputManager()->BindAxis(PitchCamera_mapping, this, &ACameraActor::PitchCamera);

        InputActionMapping Fly_mapping("Fly");
        Fly_mapping.AddGroup(InputKey::KEY_MOUSE_RIGHT);
        GetInputManager()->BindAction(Fly_mapping, InputEvent::IE_Pressed, this, &ACameraActor::BeginFly);
        GetInputManager()->BindAction(Fly_mapping, InputEvent::IE_Released, this, &ACameraActor::EndFly);

        InputActionMapping LmbNav_mapping("LmbNav");
        LmbNav_mapping.AddGroup(InputKey::KEY_MOUSE_LEFT);
        GetInputManager()->BindAction(LmbNav_mapping, InputEvent::IE_Pressed, this, &ACameraActor::BeginLmbNav);
        GetInputManager()->BindAction(LmbNav_mapping, InputEvent::IE_Released, this, &ACameraActor::EndLmbNav);

        InputActionMapping ToOrigin_mapping("ToOrigin");
        ToOrigin_mapping.AddGroup(InputKey::KEY_KP_0);
        GetInputManager()->BindAction(ToOrigin_mapping, InputEvent::IE_Pressed, this, &ACameraActor::ToOrigin);

        InputAxisMapping SpeedUp_mapping("SpeedUp");
        SpeedUp_mapping.AddGroup(InputKey::KEY_PAGEUP, 1.0f);
        SpeedUp_mapping.AddGroup(InputKey::KEY_PAGEDOWN, -1.0f);
        GetInputManager()->BindAxis(SpeedUp_mapping, this, &ACameraActor::SpeedUp);

        InputAxisMapping WheelSpeed_mapping("WheelSpeed");
        WheelSpeed_mapping.AddGroup(InputKey::AXIS_MOUSEWHEEL, 1.0f);
        GetInputManager()->BindAxis(WheelSpeed_mapping, this, &ACameraActor::AdjustMovementSpeedByWheel);
    }

    void ACameraActor::Tick(double DeltaTime)
    {
        AActor::Tick(DeltaTime);

        CameraComponent->VerticalFieldOfView = glm::radians(50.0);

        if (bIsLmbNavigating)
        {
            {
                FRotator NewRotation = GetActorRotator();
                NewRotation.Yaw += LmbInput.x * MouseSensitivity;
                SetActorRotator(NewRotation);
            }

            {
                FVector Forward = GetActorForwardVector();
                FVector DollyDir(Forward.x, Forward.y, 0.0);
                double DollyLen = glm::length(DollyDir);
                if (DollyLen > KINDA_SMALL_NUMBER)
                {
                    DollyDir /= DollyLen;
                    FVector NewLocation = GetActorLocation();
                    NewLocation += DollyDir * (LmbInput.y * DollySensitivity);
                    SetActorLocation(NewLocation);
                }
            }
        }

        if (bIsFlying)
        {
            {
                FRotator NewRotation = GetActorRotator();      
                NewRotation.Yaw += CameraInput.x * MouseSensitivity;
                SetActorRotator(NewRotation);
            }

            {
                FRotator NewRotation = GetActorRotator();
                NewRotation.Pitch = glm::clamp(NewRotation.Pitch + CameraInput.y * MouseSensitivity, -80.0, 80.0);
                SetActorRotator(NewRotation);
            }

            {
                FVector NewLocation = GetActorLocation();
                FVector forward = GetActorForwardVector();
                FVector right = GetActorRightVector();
                NewLocation += forward * MovementInput.x * DeltaTime * double(MovementSpeed);
                NewLocation += right * MovementInput.y * DeltaTime * double(MovementSpeed);
                NewLocation += WORLD_UP * (VerticalMovementInput * DeltaTime * double(MovementSpeed));
                SetActorLocation(NewLocation);
            }
        }

        MovementInput = CameraInput = LmbInput = FVector2(0.f, 0.f);
        VerticalMovementInput = 0.f;
    }

    FSceneView ACameraActor::CalcSceneView(const FSceneViewFamily& ViewFamily)
    {
        FSceneView SceneView = FSceneView(
            CameraComponent->ProjectionMode,
            CameraComponent->VerticalFieldOfView, 
            CameraComponent->OrthoWidth,
            CameraComponent->NearClipDistance, 
            CameraComponent->FarClipDistance, 
            CameraComponent->GetComponentLocation(), 
            CameraComponent->GetForwardVector(), 
            CameraComponent->GetUpVector(), 
            FIntVector2(ViewFamily.Viewport.Width, ViewFamily.Viewport.Height));

        return SceneView;
    }

    void ACameraActor::MoveForward(float AxisValue)
    {
        if (!bIsFlying)
            return;
        MovementInput.x = AxisValue;
    }

    void ACameraActor::MoveRight(float AxisValue)
    {
        if (!bIsFlying)
            return;
        MovementInput.y = AxisValue;
    }

    void ACameraActor::MoveUp(float AxisValue)
    {
        if (!bIsFlying)
            return;
        VerticalMovementInput = AxisValue;
    }

    void ACameraActor::PitchCamera(float AxisValue)
    {
        if (bIsFlying)
            CameraInput.y += AxisValue;
        else if (bIsLmbNavigating)
            LmbInput.y += AxisValue;
    }

    void ACameraActor::YawCamera(float AxisValue)
    {
        if (bIsFlying)
            CameraInput.x += AxisValue;
        else if (bIsLmbNavigating)
            LmbInput.x += AxisValue;
    }

    void ACameraActor::UpdateCursorCapture()
    {
        GApplication->SetCursorCaptured(bIsFlying || bIsLmbNavigating);
    }

    void ACameraActor::BeginFly()
    {
        if (bIsLmbNavigating)
            return;
        bIsFlying = true;
        CameraInput = LmbInput = FVector2(0.f, 0.f);
        UpdateCursorCapture();
    }

    void ACameraActor::EndFly()
    {
        bIsFlying = false;
        MovementInput = CameraInput = FVector2(0.f, 0.f);
        VerticalMovementInput = 0.f;
        UpdateCursorCapture();
    }

    void ACameraActor::BeginLmbNav()
    {
        if (bIsFlying)
            return;
        bIsLmbNavigating = true;
        CameraInput = LmbInput = FVector2(0.f, 0.f);
        UpdateCursorCapture();
    }

    void ACameraActor::EndLmbNav()
    {
        bIsLmbNavigating = false;
        LmbInput = FVector2(0.f, 0.f);
        UpdateCursorCapture();
    }

    void ACameraActor::ToOrigin()
    {
        if (!bIsFlying)
            return;
        CameraComponent->SetWorldLocation(FVector(10, 10, 0));
    }

    void ACameraActor::SpeedUp(float AxisValue)
    {
        MovementSpeed += AxisValue * 5;
        MovementSpeed = std::max(MovementSpeed, 1.f);
    }

    void ACameraActor::AdjustMovementSpeedByWheel(float AxisValue)
    {
        if (!bIsFlying)
            return;
        MovementSpeed += AxisValue * MouseWheelSpeedStep;
        MovementSpeed = std::max(MovementSpeed, 1.f);
    }
}
