#pragma once

// #include <glm/glm.hpp>
#include "Common/Transform.h"

namespace nilou {

    enum class ECullingResult
    {
        CR_Outside = -1,
        CR_Intersecting = 0,
        CR_Inside = 1
    };

    /**
    * @brief A plane in Hessian Normal Format.
    */
    class FPlane 
    {
    public:
        /**
        * @brief Constructs a new plane with a +Z normal and a distance of 0.0.
        */
        FPlane() noexcept;

        /**
        * @brief Constructs a new plane from a normal and a distance from the origin.
        *
        * The plane is defined by:
        * ```
        * ax + by + cz + d = 0
        * ```
        * where (a, b, c) is the plane's `normal`, d is the signed
        * `distance` to the plane, and (x, y, z) is any point on
        * the plane.
        *
        * @param normal The plane's normal (normalized).
        * @param distance The shortest distance from the origin to the plane. The
        * sign of `distance` determines which side of the plane the origin is on. If
        * `distance` is positive, the origin is in the half-space in the direction of
        * the normal; if negative, the origin is in the half-space opposite to the
        * normal; if zero, the plane passes through the origin.
        *
        * @exception std::exception `normal` must be normalized.
        */
        FPlane(const glm::dvec3& normal, double distance);

        /**
        * @brief Construct a new plane from a point in the plane and the plane's
        * normal.
        *
        * @param point The point on the plane.
        * @param normal The plane's normal (normalized).
        *
        * @exception std::exception `normal` must be normalized.
        *
        */
        FPlane(const glm::dvec3& point, const glm::dvec3& normal);

        /**
        * @brief Computes the signed shortest distance of a point to this plane.
        * The sign of the distance determines which side of the plane the point
        * is on.  If the distance is positive, the point is in the half-space
        * in the direction of the normal; if negative, the point is in the half-space
        * opposite to the normal; if zero, the plane passes through the point.
        *
        * @param point The point.
        * @returns The signed shortest distance of the point to the plane.
        */
        double GetPointDistance(const glm::dvec3& point) const noexcept;

        /**
        * @brief Projects a point onto this plane.
        * @param point The point to project onto the plane.
        * @returns The projected point.
        */
        glm::dvec3 ProjectPointOntoPlane(const glm::dvec3& point) const noexcept;

        bool Equals(const FPlane &Other, double epsilon=1e-6);

        /**
        * @brief The plane's normal.
        */
        glm::dvec3 Normal;

        /**
        * @brief The signed shortest distance from the origin to the plane.
        * The sign of `distance` determines which side of the plane the origin
        * is on.  If `distance` is positive, the origin is in the half-space
        * in the direction of the normal; if negative, the origin is in the
        * half-space opposite to the normal; if zero, the plane passes through the
        * origin.
        */
        double Distance;
    };

    struct FOrientedBoundingBox
    {  
        FOrientedBoundingBox() { }
        /**
        * @brief Constructs a new instance.
        *
        * @param center The center of the box.
        * @param halfAxes The three orthogonal half-axes of the bounding box.
        * Equivalently, the transformation matrix to rotate and scale a 1x1x1 cube
        * centered at the origin.
        */
        FOrientedBoundingBox(const glm::dvec3& center, const glm::dmat3& halfAxes) noexcept
            : Center(center)
            , HalfAxes(halfAxes) {}

        ECullingResult IntersectPlane(const FPlane& plane) const noexcept;

        glm::dvec3 Center;
        glm::dmat3 HalfAxes;
    };

    struct FBoundingSphere
    {  
        /**
        * @brief Construct a new instance.
        *
        * @param center The center of the bounding sphere.
        * @param radius The radius of the bounding sphere.
        */
        constexpr FBoundingSphere(const glm::dvec3& center, double radius) noexcept
            : Center(center), Radius(radius) {}

        glm::dvec3 Center;
        double Radius;
    };

    // Axis Aligned
    class FBoundingBox
    {
    public:
        glm::dvec3 Min;
        glm::dvec3 Max;
        FBoundingBox() { Min = Max = glm::dvec3(0); }
        FBoundingBox(glm::dvec3 Min, glm::dvec3 Max);
        FBoundingBox TransformBy(const FTransform &Transform);
    };

    class FViewFrustum
    {
    public:
        FPlane Planes[6];
        FViewFrustum() {};

        FViewFrustum(
            const glm::dvec3& Position,
            const glm::dvec3& Direction,
            const glm::dvec3& Up,
            const double AspecRatio,
            const double VerticalFieldOfView,
            const double NearClipDistance,
            const double FarClipDistance);
        /**
         * [DEPRECATED]
         * @brief Extract frustum planes using view and projection matrix
         * 
         * @param view World to view matrix
         * @param projection View to clip matrix
         * 
         * Extraction method see "Fast Extraction of Viewing Frustum Planes from the World-View-Projection Matrix"
         */
        FViewFrustum(const glm::dmat4 &view, const glm::dmat4 &projection);

        const FPlane &GetLeftPlane() const { return Planes[0]; }
        const FPlane &GetRightPlane() const { return Planes[1]; }
        const FPlane &GetTopPlane() const { return Planes[2]; }
        const FPlane &GetBottomPlane() const { return Planes[3]; }
        const FPlane &GetNearPlane() const { return Planes[4]; }
        const FPlane &GetFarPlane() const { return Planes[5]; }

        bool Intersects(const FViewFrustum &Other) const;
        bool IsBoxOutSidePlane(const FPlane &plane, const FBoundingBox &AABB) const;
        bool IsBoxOutSidePlane(const FPlane &plane, const FOrientedBoundingBox &OBB) const;
        
        /**
         * @brief Calculate if the oriented bounding box is out of frustum
         */
        bool IsBoxOutSideFrustum(const FBoundingBox &AABB) const;

        /**
         * @brief Calculate if the oriented bounding box is out of frustum
         */
        bool IsBoxOutSideFrustum(const FOrientedBoundingBox &OBB) const;

        /**
         * @brief Calculate if the axis-aligned bounding box is out of frustum, 
         * ignoring near and far clip plane to get faster.
         */
        bool IsBoxOutSideFrustumFast(const FBoundingBox &AABB) const;

        /**
         * @brief Calculate if the oriented bounding box is out of frustum, 
         * ignoring near and far clip plane to get faster.
         */
        bool IsBoxOutSideFrustumFast(const FOrientedBoundingBox &OBB) const;

        bool operator==(const FViewFrustum &Other);
    private:
        bool IsOutSidePlane(const FPlane &plane, glm::dvec3 position) const;
    };

}