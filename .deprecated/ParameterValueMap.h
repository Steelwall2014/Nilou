#pragma once
#include <utility>
#include <glm/glm.hpp>

#include "Common/SceneObject/SceneObjectTexture.h"
#include "Common/UTexture.h"

namespace und {
    template <typename T>
    struct ParameterValueMap {
        T Value;
        std::shared_ptr<SceneObjectTexture> ValueMap;

        ParameterValueMap() = default;
        ~ParameterValueMap() = default;

        explicit ParameterValueMap(const T value)
            : Value(value), ValueMap(nullptr) {};
        explicit ParameterValueMap(std::shared_ptr<SceneObjectTexture> value)
            : ValueMap(std::move(value)) {};

        ParameterValueMap &operator=(
            const std::shared_ptr<SceneObjectTexture> &rhs) {
            ValueMap = rhs;
            return *this;
        };

        ParameterValueMap &operator=(std::shared_ptr<SceneObjectTexture> &&rhs) {
            ValueMap.swap(rhs);
            return *this;
        };

        friend std::ostream &operator<<(std::ostream &out,
            const ParameterValueMap<T> &obj) {
            out << "Parameter Value: " << obj.Value << std::endl;
            if (obj.ValueMap) {
                out << "Parameter Map: " << *obj.ValueMap << std::endl;
            }

            return out;
        }
    };

    template <typename T>
    struct FParameterValueMap {
        T Value;
        std::shared_ptr<UTexture> ValueMap;

        FParameterValueMap() = default;
        ~FParameterValueMap() = default;

        explicit FParameterValueMap(const T value)
            : Value(value), ValueMap(nullptr) {};
        explicit FParameterValueMap(std::shared_ptr<UTexture> value)
            : ValueMap(std::move(value)) {};

        FParameterValueMap &operator=(
            const std::shared_ptr<UTexture> &rhs) {
            ValueMap = rhs;
            return *this;
        };

        FParameterValueMap &operator=(std::shared_ptr<UTexture> &&rhs) {
            ValueMap.swap(rhs);
            return *this;
        };

        friend std::ostream &operator<<(std::ostream &out,
            const FParameterValueMap<T> &obj) {
            out << "Parameter Value: " << obj.Value << std::endl;
            if (obj.ValueMap) {
                out << "Parameter Map: " << *obj.ValueMap << std::endl;
            }

            return out;
        }
    };

    //using Color = ParameterValueMap<glm::vec4>;
    //using Normal = ParameterValueMap<glm::vec3>;
    //using Parameter = ParameterValueMap<float>;
}
