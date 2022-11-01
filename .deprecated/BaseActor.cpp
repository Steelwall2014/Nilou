#include "BaseActor.h"
#ifdef _DEBUG
#include "Common/DebugHelper.h"
#endif // _DEBUG


glm::quat und::BaseActor::GetActorRotation()
{
    return m_pRootSceneNode->GetWorldTransform().GetRotation();
}
und::Rotator und::BaseActor::GetActorRotator()
{
    glm::quat rotation = m_pRootSceneNode->GetWorldTransform().GetRotation();
    und::Rotator output;
    output.Yaw = glm::yaw(rotation);
    output.Pitch = glm::pitch(rotation);
    output.Roll = glm::roll(rotation);
    return output;
}
glm::vec3 und::BaseActor::GetActorLocation()
{
    return m_pRootSceneNode->GetNodeLocation();
}

glm::vec3 und::BaseActor::GetActorForwardVector()
{
    return m_pRootSceneNode->GetForwardVector();
}

glm::vec3 und::BaseActor::GetActorUpVector()
{
    return m_pRootSceneNode->GetUpVector();
}

glm::vec3 und::BaseActor::GetActorRightVector()
{
    return m_pRootSceneNode->GetRightVector();
}

void und::BaseActor::SetActorRotation(const glm::quat &rotation)
{
#ifdef _DEBUG
    //UNDDEBUG_PrintGLM(rotation);
    //UNDDEBUG_PrintGLM(und::Rotator(rotation));
#endif

    m_pRootSceneNode->MoveNode(glm::vec3(0, 0, 0), rotation);
}

void und::BaseActor::SetActorRotator(const und::Rotator &rotator)
{
    m_pRootSceneNode->MoveNode(glm::vec3(0, 0, 0), rotator);
    //SetActorRotation(glm::quat(glm::vec3(rotator.Pitch, rotator.Yaw, rotator.Roll)));
}

void und::BaseActor::SetActorLocation(const glm::vec3 &location)
{
     glm::vec3 delta = location - m_pRootSceneNode->GetNodeLocation();
    //std::cout << location.x << " " << location.y << " " << location.z << std::endl;
    m_pRootSceneNode->MoveNode(delta, m_pRootSceneNode->GetWorldTransform().GetRotation());
}

std::shared_ptr<und::BaseSceneNode> und::BaseActor::GetRootSceneNode()
{
    return m_pRootSceneNode;
}
