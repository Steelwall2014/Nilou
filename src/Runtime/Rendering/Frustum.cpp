#include "Frustum.h"

namespace nilou {

    FBoundingBox::FBoundingBox(glm::vec3 Min, glm::vec3 Max)
        : Min(Min)
        , Max(Max)
    {
    }

    FBoundingBox FBoundingBox::TransformBy(const FTransform &Transform)
    {
        FBoundingBox NewBox;
        glm::mat4 M = Transform.ToMatrix();

        NewBox.Min = NewBox.Max = Transform.GetTranslation();

        {   // NewBox.Min.x NewBox.Max.x
            if (M[0][0] > 0.f)
            {
                NewBox.Min.x += M[0][0] * Min.x;
                NewBox.Max.x += M[0][0] * Max.x;
            }
            else 
            {
                NewBox.Min.x += M[0][0] * Max.x;
                NewBox.Max.x += M[0][0] * Min.x;
            }
            if (M[1][0] > 0.f)
            {
                NewBox.Min.x += M[1][0] * Min.y;
                NewBox.Max.x += M[1][0] * Max.y;
            }
            else 
            {
                NewBox.Min.x += M[1][0] * Max.y;
                NewBox.Max.x += M[1][0] * Min.y;
            }
            if (M[2][0] > 0.f)
            {
                NewBox.Min.x += M[2][0] * Min.z;
                NewBox.Max.x += M[2][0] * Max.z;
            }
            else 
            {
                NewBox.Min.x += M[2][0] * Max.z;
                NewBox.Max.x += M[2][0] * Min.z;
            }
        }

        {   // NewBox.Min.y NewBox.Max.y
            if (M[0][1] > 0.f)
            {
                NewBox.Min.y += M[0][1] * Min.x;
                NewBox.Max.y += M[0][1] * Max.x;
            }
            else 
            {
                NewBox.Min.y += M[0][1] * Max.x;
                NewBox.Max.y += M[0][1] * Min.x;
            }
            if (M[1][1] > 0.f)
            {
                NewBox.Min.y += M[1][1] * Min.y;
                NewBox.Max.y += M[1][1] * Max.y;
            }
            else 
            {
                NewBox.Min.y += M[1][1] * Max.y;
                NewBox.Max.y += M[1][1] * Min.y;
            }
            if (M[2][1] > 0.f)
            {
                NewBox.Min.y += M[2][1] * Min.z;
                NewBox.Max.y += M[2][1] * Max.z;
            }
            else 
            {
                NewBox.Min.y += M[2][1] * Max.z;
                NewBox.Max.y += M[2][1] * Min.z;
            }
        }

        {   // NewBox.Min.z NewBox.Max.z
            if (M[0][2] > 0.f)
            {
                NewBox.Min.z += M[0][2] * Min.x;
                NewBox.Max.z += M[0][2] * Max.x;
            }
            else 
            {
                NewBox.Min.z += M[0][2] * Max.x;
                NewBox.Max.z += M[0][2] * Min.x;
            }
            if (M[1][2] > 0.f)
            {
                NewBox.Min.z += M[1][2] * Min.y;
                NewBox.Max.z += M[1][2] * Max.y;
            }
            else 
            {
                NewBox.Min.z += M[1][2] * Max.y;
                NewBox.Max.z += M[1][2] * Min.y;
            }
            if (M[2][2] > 0.f)
            {
                NewBox.Min.z += M[2][2] * Min.z;
                NewBox.Max.z += M[2][2] * Max.z;
            }
            else 
            {
                NewBox.Min.z += M[2][2] * Max.z;
                NewBox.Max.z += M[2][2] * Min.z;
            }
        }
        return NewBox;
    }

    void NormalizePlane(glm::vec4 &plane)
    {
        float mag;
        mag = sqrt(plane.x * plane.x + plane.y * plane.y + plane.z * plane.z);
        plane.x = plane.x / mag;
        plane.y = plane.y / mag;
        plane.z = plane.z / mag;
        plane.w = plane.w / mag;
    }

    FViewFrustum::FViewFrustum(const glm::mat4 &view, const glm::mat4 &projection)
    {
        glm::mat4 VP = projection * view;
        VP = glm::transpose(VP);
        Planes[0].x = VP[3][0] + VP[0][0];
        Planes[0].y = VP[3][1] + VP[0][1];
        Planes[0].z = VP[3][2] + VP[0][2];
        Planes[0].w = VP[3][3] + VP[0][3];

        Planes[1].x = VP[3][0] - VP[0][0];
        Planes[1].y = VP[3][1] - VP[0][1];
        Planes[1].z = VP[3][2] - VP[0][2];
        Planes[1].w = VP[3][3] - VP[0][3];

        Planes[2].x = VP[3][0] - VP[1][0];
        Planes[2].y = VP[3][1] - VP[1][1];
        Planes[2].z = VP[3][2] - VP[1][2];
        Planes[2].w = VP[3][3] - VP[1][3];

        Planes[3].x = VP[3][0] + VP[1][0];
        Planes[3].y = VP[3][1] + VP[1][1];
        Planes[3].z = VP[3][2] + VP[1][2];
        Planes[3].w = VP[3][3] + VP[1][3];

        Planes[4].x = VP[3][0] + VP[2][0];
        Planes[4].y = VP[3][1] + VP[2][1];
        Planes[4].z = VP[3][2] + VP[2][2];
        Planes[4].w = VP[3][3] + VP[2][3];

        Planes[5].x = VP[3][0] - VP[2][0];
        Planes[5].y = VP[3][1] - VP[2][1];
        Planes[5].z = VP[3][2] - VP[2][2];
        Planes[5].w = VP[3][3] - VP[2][3];

    }
        
    bool FViewFrustum::Intersects(const FViewFrustum &Other)
    {
        // TODO
        return true;
    }

    bool FViewFrustum::IsBoxOutSidePlane(const glm::vec4 &plane, const FBoundingBox &AABB)
    {
        return 
            IsOutSidePlane(plane, glm::vec3(AABB.Min.x, AABB.Min.y, AABB.Min.z)) && 
            IsOutSidePlane(plane, glm::vec3(AABB.Min.x, AABB.Min.y, AABB.Max.z)) &&
            IsOutSidePlane(plane, glm::vec3(AABB.Min.x, AABB.Max.y, AABB.Min.z)) &&
            IsOutSidePlane(plane, glm::vec3(AABB.Min.x, AABB.Max.y, AABB.Max.z)) &&
            IsOutSidePlane(plane, glm::vec3(AABB.Max.x, AABB.Min.y, AABB.Min.z)) &&
            IsOutSidePlane(plane, glm::vec3(AABB.Max.x, AABB.Min.y, AABB.Max.z)) &&
            IsOutSidePlane(plane, glm::vec3(AABB.Max.x, AABB.Max.y, AABB.Min.z)) &&
            IsOutSidePlane(plane, glm::vec3(AABB.Max.x, AABB.Max.y, AABB.Max.z));
    }
    bool FViewFrustum::IsBoxOutSideFrustum(const FBoundingBox &AABB)
    {
        return 
            IsBoxOutSidePlane(Planes[0], AABB) || 
            IsBoxOutSidePlane(Planes[1], AABB) || 
            IsBoxOutSidePlane(Planes[2], AABB) || 
            IsBoxOutSidePlane(Planes[3], AABB) || 
            IsBoxOutSidePlane(Planes[4], AABB) || 
            IsBoxOutSidePlane(Planes[5], AABB);
    }

    // bool FViewFrustum::IsOutSideFrustum(glm::vec3 position)
    // {
    //     return IsOutSidePlane(Planes[0], position) ||
    //         IsOutSidePlane(Planes[1], position) ||
    //         IsOutSidePlane(Planes[2], position) ||
    //         IsOutSidePlane(Planes[3], position) ||
    //         IsOutSidePlane(Planes[4], position) ||
    //         IsOutSidePlane(Planes[5], position);
    // };

    bool FViewFrustum::IsOutSidePlane(glm::vec4 plane, glm::vec3 position)
    {
        // float a = glm::dot(glm::vec3(plane), position);
        return glm::dot(glm::vec3(plane), position) + plane.w < 0;
    }

}