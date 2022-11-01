#pragma once
#include "Common/BaseSceneObject.h"

namespace und {
    class SceneObjectCamera : public BaseSceneObject
    {
    public:
        float FieldOfView;
        float NearClipPlane;
        float FarClipPlane;
        float AspectRatio;

    public:        
        SceneObjectCamera(float fov, float nearclip, float farclip, float aspect_ratio);
        
        //glm::mat4 GetViewMatrix();
        //friend std::ostream &operator<<(std::ostream &out, const SceneObjectCamera &obj)
        //{
        //    out << static_cast<const BaseSceneObject &>(obj) << std::endl;
        //    out << "Aspect: " << obj.m_fAspect << std::endl;
        //    out << "Near Clip Distance: " << obj.m_fNearClipDistance << std::endl;
        //    out << "Far Clip Distance: " << obj.m_fFarClipDistance << std::endl;

        //    return out;
        //}    
        // calculates the front vector from the Camera's (updated) Euler Angles

        //bool isDirty();
    };
}