#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

#include "ObserverActor.h"
#include "Common/SceneManager.h"
#include "Common/InputManager.h"
#include "Interface/IApplication.h"
#include "Common/WorldVectors.h"

#ifdef _DEBUG
#include "Common/DebugHelper.h"
#endif // _DEBUG

namespace und {
	extern IApplication *g_pApp;
}

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
glm::vec3 SpringArm = SPRING_ARM_LENGTH * -und::WORLD_FORWARD;

double accumulate_pitch = CAMERA_PITCH;

//glm::vec3 GetCameraCoordOnSphere(float radius, float pitch, float yaw)
//{
//	float delta_y = -radius * glm::sin(pitch);
//	float delta_x = radius * glm::sin(yaw) * glm::cos(pitch);
//	float delta_z = radius * glm::cos(yaw) * glm::cos(pitch);
//	return glm::vec3(delta_x, delta_y, delta_z);
//}

und::ObserverActor::ObserverActor()
{
	CAMERA_RELATIVE_ROTATION = glm::rotate(CAMERA_RELATIVE_ROTATION, glm::radians(CAMERA_PITCH), und::WORLD_RIGHT);
	CAMERA_RELATIVE_ROTATION = glm::rotate(CAMERA_RELATIVE_ROTATION, glm::radians(CAMERA_YAW), glm::inverse(CAMERA_RELATIVE_ROTATION) * und::WORLD_UP);
	CAMERA_RELATIVE_TRANSLATION = CAMERA_RELATIVE_ROTATION * SpringArm;

	MovementSpeed = MOVEMENTSPEED;
	glm::vec3 identity_scale(1, 1, 1);
	//glm::quat root_rotation;
	////UNDDEBUG_PrintGLM(und::Rotator(rotation));
	//glm::vec3 root_location(0.f, 0.f, 5.f);
	SceneObjectTransform root_trans(identity_scale, ROOT_ROTATION, ROOT_TRANSLATION);
	m_pRootSceneNode = std::make_shared<SceneEmptyNode>();
	m_pRootSceneNode->SetRelativeTransform(root_trans);

	float screenWidth = g_pApp->GetConfiguration().screenWidth;
	float screenHeight = g_pApp->GetConfiguration().screenHeight;
	m_pCameraNode = std::make_shared<SceneCameraNode>(CAMERA_FOVY, CAMERA_NEARCLIP, CAMERA_FARCLIP, screenWidth / screenHeight);
	m_pRootSceneNode->AppendChild(m_pCameraNode);

	//glm::quat camera_rotation;
	//camera_rotation = glm::rotate(camera_rotation, glm::radians(90.0f), glm::vec3(0.f, 1.f, 0.f));
	//camera_rotation = glm::rotate(camera_rotation, glm::radians(1.0f), glm::vec3(1.f, 0.f, 0.f));
	//und::Rotator eulerAngles(camera_rotation);
	//glm::vec3 camera_translation = GetCameraCoordOnSphere(SPRING_ARM_LENGTH, eulerAngles.Pitch, eulerAngles.Yaw);
	//SceneObjectTransform transform (identity_scale, camera_rotation, camera_translation);
	SceneObjectTransform camera_transform(identity_scale, CAMERA_RELATIVE_ROTATION, CAMERA_RELATIVE_TRANSLATION);
	m_pCameraNode->SetRelativeTransform(camera_transform);

	InputAxisMapping MoveForward_mapping("MoveForward");
	MoveForward_mapping.AddGroup(InputKey::KEY_W, 1.0f);
	MoveForward_mapping.AddGroup(InputKey::KEY_S, -1.0f);
	g_pInputManager->BindAxis(MoveForward_mapping, this, &ObserverActor::MoveForward);

	InputAxisMapping MoveRight_mapping("MoveRight");
	MoveRight_mapping.AddGroup(InputKey::KEY_D, 1.0f);
	MoveRight_mapping.AddGroup(InputKey::KEY_A, -1.0f);
	g_pInputManager->BindAxis(MoveRight_mapping, this, &ObserverActor::MoveRight);

	InputAxisMapping YawCamera_mapping("YawCamera");
	YawCamera_mapping.AddGroup(InputKey::KEY_RIGHT, 1.0f);
	YawCamera_mapping.AddGroup(InputKey::KEY_LEFT, -1.0f);
	YawCamera_mapping.AddGroup(InputKey::KEY_MOUSEX, 1.0f);
	g_pInputManager->BindAxis(YawCamera_mapping, this, &ObserverActor::YawCamera);

	InputAxisMapping PitchCamera_mapping("PitchCamera");
	PitchCamera_mapping.AddGroup(InputKey::KEY_UP, 1.0f);
	PitchCamera_mapping.AddGroup(InputKey::KEY_DOWN, -1.0f);
	PitchCamera_mapping.AddGroup(InputKey::KEY_MOUSEY, 1.0f);
	g_pInputManager->BindAxis(PitchCamera_mapping, this, &ObserverActor::PitchCamera);

	InputActionMapping Zoom_mapping("Zoom");
	Zoom_mapping.AddGroup(InputKey::KEY_MOUSE_RIGHT);
	g_pInputManager->BindAction(Zoom_mapping, InputEvent::IE_Pressed, this, &ObserverActor::Zoom);

	InputActionMapping ToOrigin_mapping("ToOrigin");
	ToOrigin_mapping.AddGroup(InputKey::KEY_KP_0);
	g_pInputManager->BindAction(ToOrigin_mapping, InputEvent::IE_Pressed, this, &ObserverActor::ToOrigin);

	InputAxisMapping SpeedUp_mapping("SpeedUp");
	SpeedUp_mapping.AddGroup(InputKey::KEY_PAGEUP, 1.0f);
	SpeedUp_mapping.AddGroup(InputKey::KEY_PAGEDOWN, -1.0f);
	g_pInputManager->BindAxis(SpeedUp_mapping, this, &ObserverActor::SpeedUp);
	
}

void und::ObserverActor::Tick(double DeltaTime)
{
	BaseActor::Tick(DeltaTime);

    if (bZoomingIn)
    {
		m_pCameraNode->SetFieldOfView(2);
    }
    else
    {
		m_pCameraNode->SetFieldOfView(CAMERA_FOVY);
    }

	glm::quat NewRotation = m_pCameraNode->GetWorldTransform().GetRotation();

	if (CameraInput.y > 0.001 || CameraInput.y < -0.001)
	{
		float try_pitch = accumulate_pitch + CameraInput.y * MouseSensitivity;
		if (-85.0f < try_pitch && try_pitch < 85.0f)
		{
			accumulate_pitch = try_pitch;
			NewRotation = glm::rotate(NewRotation, glm::radians(CameraInput.y * MouseSensitivity), und::WORLD_RIGHT);
		}
	}
	if (CameraInput.x > 0.001 || CameraInput.x < -0.001)
	{
		NewRotation = glm::rotate(NewRotation, glm::radians(-CameraInput.x * MouseSensitivity), glm::inverse(NewRotation) * und::WORLD_UP);
	}

	// 如果相机到view target的距离大于一定值再计算相机的位置
	if (SPRING_ARM_LENGTH > 0.01)
	{
		// 这里一开始考虑的是把四元数变换为欧拉角，再计算相机在球面上的坐标，
		// 后来发现由于float精度问题，转换到欧拉角再计算坐标会把精度问题放大很多，导致出现相机的抖动
		// 因此修改成了用相对旋转直接计算
		glm::vec3 sphere_coord = NewRotation * SpringArm;
		glm::vec3 delta = m_pRootSceneNode->GetNodeLocation() + sphere_coord - m_pCameraNode->GetNodeLocation();
		m_pCameraNode->MoveNode(delta, NewRotation);
	}
	else
	{
		m_pCameraNode->MoveNode(glm::vec3(), NewRotation);
	}

	glm::vec3 NewLocation = GetActorLocation();
	float speed = sqrt(MovementInput.x * MovementInput.x + MovementInput.y * MovementInput.y) * MovementSpeed;
	ImGui::Text("Speed: %.6f m/s", speed);
	NewLocation += m_pCameraNode->GetForwardVector() * MovementInput.x * (float)DeltaTime * MovementSpeed;
	NewLocation += m_pCameraNode->GetRightVector() * MovementInput.y * (float)DeltaTime * MovementSpeed;
	SetActorLocation(NewLocation);

	MovementInput = CameraInput = glm::vec2(0.f, 0.f);
}


void und::ObserverActor::MoveForward(float AxisValue)
{
	MovementInput.x = AxisValue;
}

void und::ObserverActor::MoveRight(float AxisValue)
{
	MovementInput.y = AxisValue;
}

void und::ObserverActor::PitchCamera(float AxisValue)
{
	if (g_pApp->IsCursorEnabled())
		return;
	CameraInput.y += AxisValue;
}

void und::ObserverActor::YawCamera(float AxisValue)
{
	if (g_pApp->IsCursorEnabled())
		return;
	CameraInput.x += AxisValue;
}

void und::ObserverActor::Zoom()
{
	bZoomingIn = !bZoomingIn;
}

void und::ObserverActor::ToOrigin()
{
	if (g_pApp->IsCursorEnabled())
		return;
	m_pRootSceneNode->MoveNodeTo(glm::vec3(10, 10, 0));
}

void und::ObserverActor::SpeedUp(float AxisValue)
{
	MovementSpeed += AxisValue * 5;
	MovementSpeed = std::max(MovementSpeed, 0.f);
}

glm::mat4 und::ObserverActor::GetViewMatrix()
{
	return m_pCameraNode->GetViewMatrix();
}

glm::mat4 und::ObserverActor::GetProjectionMatrix()
{
	return m_pCameraNode->GetProjectionMatrix();
}

glm::vec3 und::ObserverActor::GetCameraLocation()
{
	return m_pCameraNode->GetWorldTransform().GetTranslation();
}

glm::mat4 und::ObserverActor::UNDDEBUG_GetWorldTransform()
{
	return m_pCameraNode->GetWorldTransform().ToMatrix();
}

void und::ObserverActor::SetCameraAspectRatio(float aspect_ratio)
{
	m_pCameraNode->SetAspectRatio(aspect_ratio);
}

