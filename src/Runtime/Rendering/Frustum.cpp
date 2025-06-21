#include "Frustum.h"

#include "Common/Log.h"
#include "Common/Math/BoxSphereBounds.h"

namespace nilou {

    ECullingResult FOrientedBoundingBox::IntersectPlane(const FPlane& plane) const noexcept 
    {
        const glm::dvec3 normal = plane.Normal;

        const glm::dvec3& boxVertex1 = HalfAxes[0] + HalfAxes[1] + HalfAxes[2];
        const glm::dvec3& boxVertex2 = -HalfAxes[0] + HalfAxes[1] + HalfAxes[2];
        const glm::dvec3& boxVertex3 = HalfAxes[0] - HalfAxes[1] + HalfAxes[2];
        const glm::dvec3& boxVertex4 = HalfAxes[0] + HalfAxes[1] - HalfAxes[2];

        // plane is used as if it is its normal; the first three components are
        // assumed to be normalized
        const double radEffective1 = glm::abs(glm::dot(normal, boxVertex1));
        const double radEffective2 = glm::abs(glm::dot(normal, boxVertex2));
        const double radEffective3 = glm::abs(glm::dot(normal, boxVertex3));
        const double radEffective4 = glm::abs(glm::dot(normal, boxVertex4));

        const double radEffective = glm::max(radEffective1, glm::max(radEffective2, glm::max(radEffective3, radEffective4)));

        const double distanceToPlane = plane.GetPointDistance(Center);

        if (distanceToPlane <= -radEffective) {
            // The entire box is on the negative side of the plane normal
            return ECullingResult::CR_Outside;
        }
        if (distanceToPlane >= radEffective) {
            // The entire box is on the positive side of the plane normal
            return ECullingResult::CR_Inside;
        }
        return ECullingResult::CR_Intersecting;
    }

    FBoundingSphere FBoundingSphere::TransformBy(const dmat4 &Transform) const
    {
        dvec4 center = Transform * dvec4(Center, 1);
        FBoundingSphere NewSphere(center, Radius);
        return NewSphere;
    }

    void NormalizePlane(glm::dvec4 &plane)
    {
        float mag;
        mag = sqrt(plane.x * plane.x + plane.y * plane.y + plane.z * plane.z);
        plane.x = plane.x / mag;
        plane.y = plane.y / mag;
        plane.z = plane.z / mag;
        plane.w = plane.w / mag;
    }

    FViewFrustum::FViewFrustum(
        const glm::dvec3& Position,
        const glm::dvec3& Direction,
        const glm::dvec3& Up,
        const double AspecRatio,
        const double VerticalFieldOfView,
        const double NearClipDistance,
        const double FarClipDistance)
    {
        const double t = glm::tan(0.5 * VerticalFieldOfView);
        const double b = -t;
        const double r = t * AspecRatio;
        const double l = -r;

        const double n = 1.0;

        // TODO: this is all ported directly from CesiumJS, can probably be refactored
        // to be more efficient with GLM.

        const glm::dvec3 right = glm::cross(Direction, Up);

        glm::dvec3 nearCenter = Direction * n + Position;

        // Left plane computation
        glm::dvec3 normal = right * l + nearCenter - Position;
        normal = glm::cross(normal, Up);
        normal = glm::normalize(normal);

        Planes[0] = FPlane(normal, -glm::dot(normal, Position));

        // Right plane computation
        normal = right * r;
        normal = nearCenter + normal;
        normal = normal - Position;
        normal = glm::cross(Up, normal);
        normal = glm::normalize(normal);

        Planes[1] = FPlane(normal, -glm::dot(normal, Position));

        // Top plane computation
        normal = Up * t;
        normal = nearCenter + normal;
        normal = normal - Position;
        normal = glm::cross(normal, right);
        normal = glm::normalize(normal);

        Planes[3] = FPlane(normal, -glm::dot(normal, Position));

        // Bottom plane computation
        normal = Up * b;
        normal = nearCenter + normal;
        normal = normal - Position;
        normal = glm::cross(right, normal);
        normal = glm::normalize(normal);

        Planes[2] = FPlane(normal, -glm::dot(normal, Position));

        // Near plane computation
        normal = glm::normalize(Direction);

        Planes[4] = FPlane(normal, -glm::dot(normal, Position + NearClipDistance * Direction));

        // Far plane computation
        normal = -normal;

        Planes[5] = FPlane(normal, -glm::dot(normal, Position + FarClipDistance * Direction));
    }

    FViewFrustum::FViewFrustum(const glm::dmat4 &view, const glm::dmat4 &projection)
    {
        glm::dmat4 VP = projection * view;
        VP = glm::transpose(VP);

        /** Notice
         * In paper "Fast Extraction of Viewing Frustum Planes from the World-View-Projection Matrix",
         * the plane right below should be left cliping plane. However due to the need of fitting unreal engine
         * coordinate system using opengl, the screen is flipped left and right, so the left plane became right clipping 
         * plane, the right clipping plane ditto.
         */
        // Right clipping plane
        Planes[0].Normal.x = VP[3][0] + VP[0][0];
        Planes[0].Normal.y = VP[3][1] + VP[0][1];
        Planes[0].Normal.z = VP[3][2] + VP[0][2];
        Planes[0].Distance = (VP[3][3] + VP[0][3]);
        Planes[0].Distance /= glm::length(Planes[0].Normal);
        Planes[0].Normal = glm::normalize(Planes[0].Normal);

        // Left clipping plane
        Planes[1].Normal.x = VP[3][0] - VP[0][0];
        Planes[1].Normal.y = VP[3][1] - VP[0][1];
        Planes[1].Normal.z = VP[3][2] - VP[0][2];
        Planes[1].Distance = (VP[3][3] - VP[0][3]);
        Planes[1].Distance /= glm::length(Planes[1].Normal);
        Planes[1].Normal = glm::normalize(Planes[1].Normal);

        // Top clipping plane
        Planes[2].Normal.x = VP[3][0] - VP[1][0];
        Planes[2].Normal.y = VP[3][1] - VP[1][1];
        Planes[2].Normal.z = VP[3][2] - VP[1][2];
        Planes[2].Distance = (VP[3][3] - VP[1][3]);
        Planes[2].Distance /= glm::length(Planes[2].Normal);
        Planes[2].Normal = glm::normalize(Planes[2].Normal);

        // Bottom clipping plane
        Planes[3].Normal.x = VP[3][0] + VP[1][0];
        Planes[3].Normal.y = VP[3][1] + VP[1][1];
        Planes[3].Normal.z = VP[3][2] + VP[1][2];
        Planes[3].Distance = (VP[3][3] + VP[1][3]);
        Planes[3].Distance /= glm::length(Planes[3].Normal);
        Planes[3].Normal = glm::normalize(Planes[3].Normal);

        // Near clipping plane
        Planes[4].Normal.x = VP[3][0] + VP[2][0];
        Planes[4].Normal.y = VP[3][1] + VP[2][1];
        Planes[4].Normal.z = VP[3][2] + VP[2][2];
        Planes[4].Distance = (VP[3][3] + VP[2][3]);
        Planes[4].Distance /= glm::length(Planes[4].Normal);
        Planes[4].Normal = glm::normalize(Planes[4].Normal);

        // Far clipping plane
        Planes[5].Normal.x = VP[3][0] - VP[2][0];
        Planes[5].Normal.y = VP[3][1] - VP[2][1];
        Planes[5].Normal.z = VP[3][2] - VP[2][2];
        Planes[5].Distance = (VP[3][3] - VP[2][3]);
        Planes[5].Distance /= glm::length(Planes[5].Normal);
        Planes[5].Normal = glm::normalize(Planes[5].Normal);

    }
        
    bool FViewFrustum::Intersects(const FViewFrustum &Other) const
    {
        // TODO
        return true;
    }

    bool FViewFrustum::IsBoxOutSidePlane(const FPlane &plane, const FBoxSphereBounds &Bound) const
    {
        FBox AABB = Bound.GetBox();
        return 
            IsOutSidePlane(plane, vec3(AABB.Min.x, AABB.Min.y, AABB.Min.z)) && 
            IsOutSidePlane(plane, vec3(AABB.Min.x, AABB.Min.y, AABB.Max.z)) &&
            IsOutSidePlane(plane, vec3(AABB.Min.x, AABB.Max.y, AABB.Min.z)) &&
            IsOutSidePlane(plane, vec3(AABB.Min.x, AABB.Max.y, AABB.Max.z)) &&
            IsOutSidePlane(plane, vec3(AABB.Max.x, AABB.Min.y, AABB.Min.z)) &&
            IsOutSidePlane(plane, vec3(AABB.Max.x, AABB.Min.y, AABB.Max.z)) &&
            IsOutSidePlane(plane, vec3(AABB.Max.x, AABB.Max.y, AABB.Min.z)) &&
            IsOutSidePlane(plane, vec3(AABB.Max.x, AABB.Max.y, AABB.Max.z));
    }
    bool FViewFrustum::IsBoxOutSidePlane(const FPlane &plane, const FOrientedBoundingBox &OBB) const
    {
        return OBB.IntersectPlane(plane) == ECullingResult::CR_Outside;
    }
    bool FViewFrustum::IsBoxOutSideFrustum(const FBoxSphereBounds &AABB) const
    {
        return 
            IsBoxOutSidePlane(Planes[0], AABB) || 
            IsBoxOutSidePlane(Planes[1], AABB) || 
            IsBoxOutSidePlane(Planes[2], AABB) || 
            IsBoxOutSidePlane(Planes[3], AABB) || 
            IsBoxOutSidePlane(Planes[4], AABB) || 
            IsBoxOutSidePlane(Planes[5], AABB);
    }
    bool FViewFrustum::IsBoxOutSideFrustum(const FOrientedBoundingBox &OBB) const
    {
        return 
            IsBoxOutSidePlane(Planes[0], OBB) || 
            IsBoxOutSidePlane(Planes[1], OBB) || 
            IsBoxOutSidePlane(Planes[2], OBB) || 
            IsBoxOutSidePlane(Planes[3], OBB) || 
            IsBoxOutSidePlane(Planes[4], OBB) || 
            IsBoxOutSidePlane(Planes[5], OBB);
    }
    bool FViewFrustum::IsBoxOutSideFrustum(const dvec3 &Center, const dmat3& HalfAxes) const
    {
        FOrientedBoundingBox OBB{Center, HalfAxes};
        return IsBoxOutSideFrustum(OBB);
    }
    bool FViewFrustum::IsBoxOutSideFrustumFast(const FBoxSphereBounds &AABB) const
    {
        return 
            IsBoxOutSidePlane(Planes[0], AABB) || 
            IsBoxOutSidePlane(Planes[1], AABB) || 
            IsBoxOutSidePlane(Planes[2], AABB) || 
            IsBoxOutSidePlane(Planes[3], AABB);
    }
    bool FViewFrustum::IsBoxOutSideFrustumFast(const FOrientedBoundingBox &OBB) const
    {
        return 
            IsBoxOutSidePlane(Planes[0], OBB) || 
            IsBoxOutSidePlane(Planes[1], OBB) || 
            IsBoxOutSidePlane(Planes[2], OBB) || 
            IsBoxOutSidePlane(Planes[3], OBB);
    }
    bool FViewFrustum::IsBoxOutSideFrustumFast(const dvec3 &Center, const dmat3& HalfAxes) const
    {
        FOrientedBoundingBox OBB{Center, HalfAxes};
        return IsBoxOutSideFrustumFast(OBB);
    }
    bool FViewFrustum::operator==(const FViewFrustum &Other)
    {
        return 
            Planes[0].Equals(Other.Planes[0]) && 
            Planes[1].Equals(Other.Planes[1]) && 
            Planes[2].Equals(Other.Planes[2]) && 
            Planes[3].Equals(Other.Planes[3]) && 
            Planes[4].Equals(Other.Planes[4]) && 
            Planes[5].Equals(Other.Planes[5]);
    }

    bool FViewFrustum::IsOutSidePlane(const FPlane &plane, glm::dvec3 position) const
    {
        return glm::dot(plane.Normal, position) + plane.Distance < 0;
    }

    bool FConvexVolume::IntersectBox(const FBoxSphereBounds &Box)
    {
        for (int i = 0; i < Planes.size(); i++)
        {
            if (Box.GetBox().IntersectPlane(Planes[i]) == ECullingResult::CR_Outside)
                return false;
        }
        return true;
    }

}