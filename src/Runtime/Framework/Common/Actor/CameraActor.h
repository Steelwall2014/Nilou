#include "Actor.h"
#include "Common/Components/CameraComponent.h"

namespace nilou {

    class NCLASS ACameraActor : public AActor
    {
		GENERATED_BODY()
    public:
        ACameraActor();

        virtual void Tick(double DeltaTime);
		
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
		
		vec2 MovementInput = vec2(0, 0);
		vec2 CameraInput = vec2(0, 0);
		float CameraRollInput = 0.f;
		float ZoomFactor;
		bool bZoomingIn = false;
		float MovementSpeed = 100.0f;
		float MouseSensitivity = 0.18f;
    };

}