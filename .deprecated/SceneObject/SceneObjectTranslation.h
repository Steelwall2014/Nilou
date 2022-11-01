#pragma once
#include "SceneObjectTransform.h"

namespace und {
    class SceneObjectTranslation : public SceneObjectTransform
    {
    public:
        SceneObjectTranslation(const char axis, const float amount)
        {
            switch (axis) {
            case 'x':
                m_matrix = glm::translate(m_matrix, glm::vec3(amount, 0.0f, 0.0f));
                break;
            case 'y':
                m_matrix = glm::translate(m_matrix, glm::vec3(0.0f, amount, 0.0f));
                break;
            case 'z':
                m_matrix = glm::translate(m_matrix, glm::vec3(0.0f, 0.0f, amount));
                break;
            default:
                assert(0);
            }
        }

        SceneObjectTranslation(const float x, const float y, const float z)
        {
            m_matrix = glm::translate(m_matrix, glm::vec3(x, y, z));
        }
    };
}