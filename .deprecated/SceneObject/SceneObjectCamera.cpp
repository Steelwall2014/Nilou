#include "SceneObjectCamera.h"

//und::SceneObjectCamera::SceneObjectCamera(glm::vec3 position, glm::vec3 up, float yaw, float pitch) 
//    : BaseSceneObject(kSceneObjectTypeMesh), Front(glm::vec3(0.0f, 0.0f, -1.0f))
//{
//    Position = position;
//    WorldUp = up;
//    Yaw = yaw;
//    Pitch = pitch;
//    updateCameraVectors();
//}


und::SceneObjectCamera::SceneObjectCamera(float fov, float nearclip, float farclip, float aspect_ratio)
    : BaseSceneObject(kSceneObjectTypeCamera)
    , FieldOfView(fov)
    , NearClipPlane(nearclip)
    , FarClipPlane(farclip)
    , AspectRatio(aspect_ratio)
{
}

//glm::mat4 und::SceneObjectCamera::GetViewMatrix()
//{
//	m_bDirty = false;
//    updateCameraVectors();
//	return glm::lookAt(Position, Position + Front, Up);
//}

//void und::SceneObjectCamera::updateCameraVectors()
//{
//    // calculate the new Front vector
//    glm::vec3 front;
//    front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
//    front.y = sin(glm::radians(Pitch));
//    front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
//    Front = glm::normalize(front);
//    // also re-calculate the Right and Up vector
//    Right = glm::normalize(glm::cross(Front, WorldUp));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
//    Up = glm::normalize(glm::cross(Right, Front));
//}
//
//bool und::SceneObjectCamera::isDirty()
//{
//	return m_bDirty;
//}
