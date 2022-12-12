#include <sstream>
#include "CameraActor.h"
#include "Common/BaseApplication.h"
#include "Common/InputManager.h"

#include "Common/Log.h"

namespace nilou {

    glm::quat ROOT_ROTATION(1.f, 0.f, 0.f, 0.f);
    //glm::vec3 ROOT_TRANSLATION(10120, 10120, -20);
    glm::vec3 ROOT_TRANSLATION(10, 10, 0);
    //glm::vec3 ROOT_TRANSLATION(10000, 10000, 20);

    float MOVEMENTSPEED = 100;
    float SPRING_ARM_LENGTH = 0.f;
    float CAMERA_FOVY = 55.f;
    float CAMERA_NEARCLIP = 0.1f;
    float CAMERA_FARCLIP = 100000.f;
    float CAMERA_PITCH = 0.f;
    float CAMERA_YAW = 0.f;
    //float CAMERA_YAW = -60.f;
    glm::quat CAMERA_RELATIVE_ROTATION(1.f, 0.f, 0.f, 0.f);
    glm::vec3 CAMERA_RELATIVE_TRANSLATION(0.f, 0.f, 0.f);
    glm::vec3 SpringArm = SPRING_ARM_LENGTH * -WORLD_FORWARD;

    double accumulate_pitch = CAMERA_PITCH;

    ACameraActor::ACameraActor()
    { 
        CameraComponent = std::make_shared<UCameraComponent>(this, true); 
        CameraComponent->AttachToComponent(GetRootComponent());

        InputAxisMapping MoveForward_mapping("MoveForward");
        MoveForward_mapping.AddGroup(InputKey::KEY_W, 1.0f);
        MoveForward_mapping.AddGroup(InputKey::KEY_S, -1.0f);
        g_pInputManager->BindAxis(MoveForward_mapping, this, &ACameraActor::MoveForward);

        InputAxisMapping MoveRight_mapping("MoveRight");
        MoveRight_mapping.AddGroup(InputKey::KEY_D, 1.0f);
        MoveRight_mapping.AddGroup(InputKey::KEY_A, -1.0f);
        g_pInputManager->BindAxis(MoveRight_mapping, this, &ACameraActor::MoveRight);

        // InputAxisMapping RollClockWise_mapping("RollClockWise");
        // RollClockWise_mapping.AddGroup(InputKey::KEY_E, 1.0f);
        // RollClockWise_mapping.AddGroup(InputKey::KEY_Q, -1.0f);
        // g_pInputManager->BindAxis(RollClockWise_mapping, this, &ACameraActor::RollClockWise);

        InputAxisMapping YawCamera_mapping("YawCamera");
        YawCamera_mapping.AddGroup(InputKey::KEY_RIGHT, 1.0f);
        YawCamera_mapping.AddGroup(InputKey::KEY_LEFT, -1.0f);
        YawCamera_mapping.AddGroup(InputKey::KEY_MOUSEX, 1.0f);
        g_pInputManager->BindAxis(YawCamera_mapping, this, &ACameraActor::YawCamera);

        InputAxisMapping PitchCamera_mapping("PitchCamera");
        PitchCamera_mapping.AddGroup(InputKey::KEY_UP, 1.0f);
        PitchCamera_mapping.AddGroup(InputKey::KEY_DOWN, -1.0f);
        PitchCamera_mapping.AddGroup(InputKey::KEY_MOUSEY, 1.0f);
        g_pInputManager->BindAxis(PitchCamera_mapping, this, &ACameraActor::PitchCamera);

        InputActionMapping Zoom_mapping("Zoom");
        Zoom_mapping.AddGroup(InputKey::KEY_MOUSE_RIGHT);
        g_pInputManager->BindAction(Zoom_mapping, InputEvent::IE_Pressed, this, &ACameraActor::Zoom);

        InputActionMapping ToOrigin_mapping("ToOrigin");
        ToOrigin_mapping.AddGroup(InputKey::KEY_KP_0);
        g_pInputManager->BindAction(ToOrigin_mapping, InputEvent::IE_Pressed, this, &ACameraActor::ToOrigin);

        InputAxisMapping SpeedUp_mapping("SpeedUp");
        SpeedUp_mapping.AddGroup(InputKey::KEY_PAGEUP, 1.0f);
        SpeedUp_mapping.AddGroup(InputKey::KEY_PAGEDOWN, -1.0f);
        g_pInputManager->BindAxis(SpeedUp_mapping, this, &ACameraActor::SpeedUp);
    }

    void ACameraActor::Tick(double DeltaTime)
    {
        AActor::Tick(DeltaTime);
        std::stringstream stream;
        stream << 
            CameraComponent->GetComponentLocation() << CameraComponent->GetComponentRotation() << "\n"
            << CameraComponent->GetForwardVector() << CameraComponent->GetRightVector() << CameraComponent->GetUpVector();
        // stream << GetActorLocation() << GetActorForwardVector();
        // stream << MovementInput << CameraInput;
        // NILOU_LOG(Info, stream.str())
        if (bZoomingIn)
        {
            CameraComponent->SetFieldOfView(2);
        }
        else
        {
            CameraComponent->SetFieldOfView(CAMERA_FOVY);
        }
        // CameraInput.x = 10;
        // CameraInput.y = 10;
        {
            FRotator NewRotation = GetActorRotator();      
            NewRotation.Yaw += CameraInput.x * MouseSensitivity;
            SetActorRotator(NewRotation);
        }

        {
            FRotator NewRotation = GetActorRotator();
            NewRotation.Pitch = glm::clamp(NewRotation.Pitch + CameraInput.y*MouseSensitivity, -80.0f, 80.0f);
            SetActorRotator(NewRotation);
        }

        {
            FRotator NewRotation = GetActorRotator();
            NewRotation.Roll += CameraRollInput * (float)DeltaTime * 10;
            SetActorRotator(NewRotation);
        }

        // MovementInput.y = 1;
        {
            vec3 NewLocation = GetActorLocation();
            vec3 forward = GetActorForwardVector();
            vec3 right = GetActorRightVector();
            NewLocation += forward * MovementInput.x * (float)DeltaTime * MovementSpeed;
            NewLocation += right * MovementInput.y * (float)DeltaTime * MovementSpeed;
            SetActorLocation(NewLocation);
        }
        // glm::quat NewRotation = CameraComponent->GetComponentRotation().ToQuat();

        // if (CameraInput.y > 0.001 || CameraInput.y < -0.001)
        // {
        //     float try_pitch = accumulate_pitch + CameraInput.y * MouseSensitivity;
        //     if (-85.0f < try_pitch && try_pitch < 85.0f)
        //     {
        //         accumulate_pitch = try_pitch;
        //         NewRotation = glm::rotate(NewRotation, glm::radians(CameraInput.y * MouseSensitivity), WORLD_RIGHT);
        //     }
        // }
        // if (CameraInput.x > 0.001 || CameraInput.x < -0.001)
        // {
        //     NewRotation = glm::rotate(NewRotation, glm::radians(-CameraInput.x * MouseSensitivity), glm::inverse(NewRotation) * WORLD_UP);
        // }

        // glm::vec3 DeltaLocation = glm::vec3(0, 0, 0);
        // if (std::abs(MovementInput.x) >= KINDA_SMALL_NUMBER || std::abs(MovementInput.y) >= KINDA_SMALL_NUMBER)
        // {
        //     // float speed = sqrt(MovementInput.x * MovementInput.x + MovementInput.y * MovementInput.y) * MovementSpeed;
        //     // ImGui::Text("Speed: %.6f m/s", speed);
        //     DeltaLocation += CameraComponent->GetForwardVector() * MovementInput.x * (float)DeltaTime * MovementSpeed;
        //     DeltaLocation += CameraComponent->GetRightVector() * MovementInput.y * (float)DeltaTime * MovementSpeed;
        //     // SetActorLocation(NewLocation);
        // }
        // 如果相机到view target的距离大于一定值再计算相机的位置
        // if (SPRING_ARM_LENGTH > 0.01)
        // {
        //     // 这里一开始考虑的是把四元数变换为欧拉角，再计算相机在球面上的坐标，
        //     // 后来发现由于float精度问题，转换到欧拉角再计算坐标会把精度问题放大很多，导致出现相机的抖动
        //     // 因此修改成了用相对旋转直接计算
        //     glm::vec3 sphere_coord = NewRotation * SpringArm;
        //     glm::vec3 delta = GetRootComponent()->GetComponentLocation() + sphere_coord - CameraComponent->GetComponentLocation();
        //     CameraComponent->MoveComponent(delta, NewRotation);
        // }
        // else
        // {
            // CameraComponent->MoveComponent(glm::vec3(), NewRotation);
            // CameraComponent->MoveComponent(DeltaLocation, NewRotation);
        // }


        MovementInput = CameraInput = glm::vec2(0.f, 0.f);
        CameraRollInput = 0;
    }

    void ACameraActor::MoveForward(float AxisValue)
    {
        MovementInput.x = AxisValue;
    }

    void ACameraActor::MoveRight(float AxisValue)
    {
        MovementInput.y = AxisValue;
    }

    void ACameraActor::PitchCamera(float AxisValue)
    {
        if (GetAppication()->IsCursorEnabled())
            return;
        CameraInput.y += AxisValue;
    }

    void ACameraActor::YawCamera(float AxisValue)
    {
        if (GetAppication()->IsCursorEnabled())
            return;
        CameraInput.x += AxisValue;
    }

    void ACameraActor::RollClockWise(float AxisValue)
    {
        CameraRollInput = AxisValue;
    }

    void ACameraActor::Zoom()
    {
        bZoomingIn = !bZoomingIn;
    }

    void ACameraActor::ToOrigin()
    {
        if (GetAppication()->IsCursorEnabled())
            return;
        CameraComponent->SetWorldLocation(glm::vec3(10, 10, 0));
    }

    void ACameraActor::SpeedUp(float AxisValue)
    {
        MovementSpeed += AxisValue * 5;
        MovementSpeed = std::max(MovementSpeed, 0.f);
    }
}