#pragma once
#include "Common/BaseActor.h"
#include "Common/SceneNode.h"

namespace und {
	class ObserverActor : public BaseActor
	{
		friend class SceneManager;
	private:
		std::shared_ptr<SceneCameraNode> m_pCameraNode;
		glm::vec2 MovementInput;
		glm::vec2 CameraInput;
		float ZoomFactor;
		bool bZoomingIn = false;
		float MovementSpeed = 50.0f;
		float MouseSensitivity = 0.18f;
	public:
		ObserverActor();
		virtual void Tick(double DeltaTime);
		void MoveForward(float AxisValue);
		void MoveRight(float AxisValue);
		void PitchCamera(float AxisValue);
		void YawCamera(float AxisValue);
		void Zoom();
		void ToOrigin();
		void SpeedUp(float AxisValue);

		// ������m_pCameraNodeת��
		glm::mat4 GetViewMatrix();
		glm::mat4 GetProjectionMatrix(); 
		glm::vec3 GetCameraLocation();
		glm::mat4 UNDDEBUG_GetWorldTransform();
		void SetCameraAspectRatio(float aspect_ratio);
	};

}