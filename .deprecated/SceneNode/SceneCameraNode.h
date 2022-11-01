#pragma once
#include "Common/BaseSceneNode.h"

namespace und {
    class SceneCameraNode : public SceneNode<SceneObjectCamera>
    {
    public:
        using SceneNode::SceneNode;
        SceneCameraNode(float fov, float nearclip, float farclip, float aspect_ratio);
        void SetFieldOfView(float fov);
        void SetAspectRatio(float aspect_ratio);
        glm::mat4 GetViewMatrix();
        glm::mat4 GetProjectionMatrix();
        //bool isDirty();
    };
}