#pragma once
#include "SceneObjectTransform.h"

namespace und {
    class SceneObjectRotation : public SceneObjectTransform
    {
    public:
        SceneObjectRotation(const char axis, const float theta)
        {
            switch (axis) {
            case 'x':
                m_matrix = glm::rotate(m_matrix, theta, glm::vec3(1.0f, 0.0f, 0.0f));
                break;
            case 'y':
                m_matrix = glm::rotate(m_matrix, theta, glm::vec3(0.0f, 1.0f, 0.0f));
                break;
            case 'z':
                m_matrix = glm::rotate(m_matrix, theta, glm::vec3(0.0f, 0.0f, 1.0f));
                break;
            default:
                assert(0);
            }
        }

        SceneObjectRotation(glm::vec3 &axis, const float theta)
        {
            m_matrix = glm::rotate(m_matrix, theta, glm::normalize(axis));
        }

        SceneObjectRotation(const glm::quat quaternion)
        {
            m_matrix = glm::mat4_cast(quaternion);
        }
    };
}