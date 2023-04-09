#include "Actor.h"
#include "Common/Components/CameraComponent.h"

namespace nilou {

	UCLASS()
    class ACameraActor : public AActor
    {
		GENERATE_CLASS_INFO()
    public:
        ACameraActor();

        virtual void Tick(double DeltaTime);
		
		void SetCameraResolution(const ivec2 &CameraResolution) { CameraComponent->SetCameraResolution(CameraResolution); }

		void SetMoveSpeed(double Speed) { MovementSpeed = Speed; }

		FSceneView* CalcSceneView(FSceneViewFamily* ViewFamily);

		UCameraComponent* GetCameraComponent() const { return CameraComponent.get(); }

    protected:


		void MoveForward(float AxisValue);
		void MoveRight(float AxisValue);
		void PitchCamera(float AxisValue);
		void YawCamera(float AxisValue);
		void RollClockWise(float AxisValue);
		void Zoom();
		void ToOrigin();
		void SpeedUp(float AxisValue);

        std::shared_ptr<UCameraComponent> CameraComponent;

		/** TEST SCENE CAPTURE */
        std::shared_ptr<class USceneCaptureComponent2D> SceneCaptureComponent;
		/** TEST SCENE CAPTURE */

		/** TEST SCENE CAPTURE CUBE */
        std::shared_ptr<class USceneCaptureComponentCube> SceneCaptureComponentCube;
		/** TEST SCENE CAPTURE CUBE */
		
		glm::vec2 MovementInput = glm::vec2(0, 0);
		glm::vec2 CameraInput = glm::vec2(0, 0);
		float CameraRollInput = 0.f;
		float ZoomFactor;
		bool bZoomingIn = false;
		float MovementSpeed = 1.0f;
		float MouseSensitivity = 0.18f;
    };

}