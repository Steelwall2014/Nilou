#pragma once

// #include <glm/glm.hpp>
#include "Common/Transform.h"

namespace nilou {

    // Axis Aligned
    class FBoundingBox
    {
    public:
        glm::vec3 Min;
        glm::vec3 Max;
        FBoundingBox() { Min = Max = glm::vec3(0); }
        FBoundingBox(glm::vec3 Min, glm::vec3 Max);
        FBoundingBox TransformBy(const FTransform &Transform);
    };

    class FViewFrustum
    {
    public:
        glm::vec4 Planes[6];
        FViewFrustum(const glm::mat4 &view, const glm::mat4 &projection);
        FViewFrustum() {};
        bool Intersects(const FViewFrustum &Other);
        // bool IsOutSideFrustum(glm::vec3 position);
        bool IsBoxOutSidePlane(const glm::vec4 &plane, const FBoundingBox &);
        bool IsBoxOutSideFrustum(const FBoundingBox &);
    private:
        bool IsOutSidePlane(glm::vec4 plane, glm::vec3 position);
    };

}