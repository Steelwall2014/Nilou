#pragma once
#include "Common/BaseSceneNode.h"

namespace und {
    class SceneLightNode : public SceneNode<SceneObjectLight>
    {
    protected:
        glm::vec3 m_Target;

    public:
        using SceneNode::SceneNode;
        SceneLightNode(glm::vec3 position, glm::vec3 target);
        void SetTarget(glm::vec3 &target);
        const glm::vec3 &GetTarget() { return m_Target; };
        glm::vec3 GetLightDir();
    };

}