#include "SceneCameraNode.h"
#include "Common/WorldVectors.h"


und::SceneCameraNode::SceneCameraNode(float fov, float nearclip, float farclip, float aspect_ratio)
{
    m_NodeType = "SceneCameraNode";
    m_pSceneObject = std::make_shared<SceneObjectCamera>(fov, nearclip, farclip, aspect_ratio);
    //m_pSceneObject->FieldOfView = fov;
    //m_pSceneObject->NearClipPlane = nearclip;
    //m_pSceneObject->FarClipPlane = farclip;
    //m_pSceneObject->AspectRatio = aspect_ratio;
}

void und::SceneCameraNode::SetFieldOfView(float fov)
{
    m_pSceneObject->FieldOfView = fov;
}

void und::SceneCameraNode::SetAspectRatio(float aspect_ratio)
{
    m_pSceneObject->AspectRatio = aspect_ratio;
}

glm::mat4 und::SceneCameraNode::GetViewMatrix()
{
    //glm::vec3 Up = GetUpVector();
    glm::vec3 Front = GetForwardVector();
    glm::vec3 Position = GetWorldTransform().GetTranslation();
    return glm::lookAt(Position, Position + Front, WORLD_UP);
}

glm::mat4 und::SceneCameraNode::GetProjectionMatrix()
{
    return glm::perspective(glm::radians(m_pSceneObject->FieldOfView), m_pSceneObject->AspectRatio, m_pSceneObject->NearClipPlane, m_pSceneObject->FarClipPlane);
}
//bool und::SceneCameraNode::isDirty()
//{
//    return m_pSceneObject->isDirty();
//}
