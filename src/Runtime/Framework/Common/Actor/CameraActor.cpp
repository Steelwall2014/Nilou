#include <sstream>
#include "CameraActor.h"
#include "BaseApplication.h"
#include "Common/InputManager.h"

#include "Common/Log.h"

#include "Common/Components/SceneCaptureComponent.h"

namespace nilou {

    glm::quat ROOT_ROTATION(1.f, 0.f, 0.f, 0.f);
    //glm::vec3 ROOT_TRANSLATION(10120, 10120, -20);
    glm::vec3 ROOT_TRANSLATION(10, 10, 0);
    //glm::vec3 ROOT_TRANSLATION(10000, 10000, 20);

    float MOVEMENTSPEED = 100;
    // float SPRING_ARM_LENGTH = 0.f;
    // float CAMERA_FOVY = glm::radians(55.f);
    // float CAMERA_NEARCLIP = 0.1f;
    // float CAMERA_FARCLIP = 100000.f;
    // float CAMERA_PITCH = 0.f;
    // float CAMERA_YAW = 0.f;
    // //float CAMERA_YAW = -60.f;
    // glm::quat CAMERA_RELATIVE_ROTATION(1.f, 0.f, 0.f, 0.f);
    // glm::vec3 CAMERA_RELATIVE_TRANSLATION(0.f, 0.f, 0.f);
    // glm::vec3 SpringArm = SPRING_ARM_LENGTH * -WORLD_FORWARD;

    // double accumulate_pitch = CAMERA_PITCH;

    ACameraActor::ACameraActor()
    { 
        CameraComponent = CreateComponent<UCameraComponent>(this); 
        CameraComponent->AttachToComponent(GetRootComponent());

		/** TEST SCENE CAPTURE 2D*/
        {
            // std::shared_ptr<FImage2D> Image = std::make_shared<FImage2D>(1920, 1080, EPixelFormat::PF_R16G16B16A16F);
            // UTextureRenderTarget2D* RenderTarget = new UTextureRenderTarget2D("Test");
            // RenderTarget->ImageData = Image;
            // RenderTarget->UpdateResource();
            // SceneCaptureComponent = CreateComponent<USceneCaptureComponent2D>(this); 
            // SceneCaptureComponent->AttachToComponent(CameraComponent.get());
            // SceneCaptureComponent->TextureTarget = RenderTarget;
            // SceneCaptureComponent->VerticalFieldOfView = CameraComponent->VerticalFieldOfView;
        }
		/** TEST SCENE CAPTURE 2D*/

		/** TEST SCENE CAPTURE CUBE*/
        {
            // std::shared_ptr<FImageCube> Image = std::make_shared<FImageCube>(1024, 1024, EPixelFormat::PF_R16G16B16A16F);
            // UTextureRenderTargetCube* RenderTarget = new UTextureRenderTargetCube("Test");
            // RenderTarget->ImageData = Image;
            // RenderTarget->UpdateResource();
            // SceneCaptureComponentCube = CreateComponent<USceneCaptureComponentCube>(this); 
            // SceneCaptureComponentCube->AttachToComponent(CameraComponent.get());
            // SceneCaptureComponentCube->TextureTarget = RenderTarget;
        }
		/** TEST SCENE CAPTURE CUBE*/

        InputAxisMapping MoveForward_mapping("MoveForward");
        MoveForward_mapping.AddGroup(InputKey::KEY_W, 1.0f);
        MoveForward_mapping.AddGroup(InputKey::KEY_S, -1.0f);
        GetInputManager()->BindAxis(MoveForward_mapping, this, &ACameraActor::MoveForward);

        InputAxisMapping MoveRight_mapping("MoveRight");
        MoveRight_mapping.AddGroup(InputKey::KEY_D, 1.0f);
        MoveRight_mapping.AddGroup(InputKey::KEY_A, -1.0f);
        GetInputManager()->BindAxis(MoveRight_mapping, this, &ACameraActor::MoveRight);

        // InputAxisMapping RollClockWise_mapping("RollClockWise");
        // RollClockWise_mapping.AddGroup(InputKey::KEY_E, 1.0f);
        // RollClockWise_mapping.AddGroup(InputKey::KEY_Q, -1.0f);
        // GetInputManager()->BindAxis(RollClockWise_mapping, this, &ACameraActor::RollClockWise);

        InputAxisMapping YawCamera_mapping("YawCamera");
        YawCamera_mapping.AddGroup(InputKey::KEY_RIGHT, 1.0f);
        YawCamera_mapping.AddGroup(InputKey::KEY_LEFT, -1.0f);
        YawCamera_mapping.AddGroup(InputKey::AXIS_MOUSEX, 1.0f);
        GetInputManager()->BindAxis(YawCamera_mapping, this, &ACameraActor::YawCamera);

        InputAxisMapping PitchCamera_mapping("PitchCamera");
        PitchCamera_mapping.AddGroup(InputKey::KEY_UP, 1.0f);
        PitchCamera_mapping.AddGroup(InputKey::KEY_DOWN, -1.0f);
        PitchCamera_mapping.AddGroup(InputKey::AXIS_MOUSEY, 1.0f);
        GetInputManager()->BindAxis(PitchCamera_mapping, this, &ACameraActor::PitchCamera);

        InputActionMapping Zoom_mapping("Zoom");
        Zoom_mapping.AddGroup(InputKey::KEY_MOUSE_RIGHT);
        GetInputManager()->BindAction(Zoom_mapping, InputEvent::IE_Pressed, this, &ACameraActor::Zoom);

        InputActionMapping ToOrigin_mapping("ToOrigin");
        ToOrigin_mapping.AddGroup(InputKey::KEY_KP_0);
        GetInputManager()->BindAction(ToOrigin_mapping, InputEvent::IE_Pressed, this, &ACameraActor::ToOrigin);

        InputAxisMapping SpeedUp_mapping("SpeedUp");
        SpeedUp_mapping.AddGroup(InputKey::KEY_PAGEUP, 1.0f);
        SpeedUp_mapping.AddGroup(InputKey::KEY_PAGEDOWN, -1.0f);
        GetInputManager()->BindAxis(SpeedUp_mapping, this, &ACameraActor::SpeedUp);
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
            CameraComponent->VerticalFieldOfView = glm::radians(2.0);
        }
        else
        {
            CameraComponent->VerticalFieldOfView = glm::radians(50.0);
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
            NewRotation.Pitch = glm::clamp(NewRotation.Pitch + CameraInput.y*MouseSensitivity, -80.0, 80.0);
            SetActorRotator(NewRotation);
        }

        {
            FRotator NewRotation = GetActorRotator();
            NewRotation.Roll += CameraRollInput * DeltaTime * 10.0;
            SetActorRotator(NewRotation);
        }

        {
            vec3 NewLocation = GetActorLocation();
            vec3 forward = GetActorForwardVector();
            vec3 right = GetActorRightVector();
            NewLocation += forward * MovementInput.x * (float)DeltaTime * MovementSpeed;
            NewLocation += right * MovementInput.y * (float)DeltaTime * MovementSpeed;
            SetActorLocation(NewLocation);
        }

        MovementInput = CameraInput = glm::vec2(0.f, 0.f);
        CameraRollInput = 0;
    }

    FSceneView* ACameraActor::CalcSceneView(FSceneViewFamily* ViewFamily)
    {
        FSceneView* SceneView = new FSceneView(
            CameraComponent->ProjectionMode,
            CameraComponent->VerticalFieldOfView, 
            CameraComponent->OrthoWidth,
            CameraComponent->NearClipDistance, 
            CameraComponent->FarClipDistance, 
            CameraComponent->GetComponentLocation(), 
            CameraComponent->GetForwardVector(), 
            CameraComponent->GetUpVector(), 
            ivec2(ViewFamily->Viewport.Width, ViewFamily->Viewport.Height),
            CameraComponent->ViewUniformBuffer);
            
        auto ViewUniformBuffer = CameraComponent->ViewUniformBuffer;

        ENQUEUE_RENDER_COMMAND(ACameraActor_CalcSceneView)(
            [ViewUniformBuffer](FDynamicRHI*) 
            {
                ViewUniformBuffer->UpdateUniformBuffer();
            });

        return SceneView;
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
        MovementSpeed = std::max(MovementSpeed, 1.f);
    }
}