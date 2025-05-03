#pragma once

#include "Maths.h"
#include "Transform.h"

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
    FPlane() noexcept : FPlane(glm::dvec3(0.0, 0.0, 1.0), 0.0) {}

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
    FPlane(const glm::dvec3& normal, double distance)
        : Normal(normal), Distance(distance)
    {
        if (!FMath::equalsEpsilon(glm::length(normal), 1.0, 1e-6)) 
        {
            NILOU_LOG(Fatal, "normal must be normalized.");
            std::cout << normal << std::endl;;
        }
    }

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
    FPlane(const glm::dvec3& point, const glm::dvec3& normal) : FPlane(normal, -glm::dot(normal, point)) {}

    /**
    * @brief Construct a new plane from three points in the plane.
    *
    * @param A First point in the plane.
    * @param B Second point in the plane.
    * @param C Third point in the plane.
    *
    */
    FPlane(const glm::dvec3& A, const glm::dvec3& B, const glm::dvec3& C)
    {
        Normal = glm::normalize(glm::cross(B-A, C-A));
        Distance = -glm::dot(Normal, A);
    }

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
    double GetPointDistance(const glm::dvec3& point) const noexcept
    {
        return glm::dot(this->Normal, point) + this->Distance;
    }

    /**
    * @brief Projects a point onto this plane.
    * @param point The point to project onto the plane.
    * @returns The projected point.
    */
    glm::dvec3 ProjectPointOntoPlane(const glm::dvec3& point) const noexcept
    {
        const double pointDistance = this->GetPointDistance(point);
        const glm::dvec3 scaledNormal = this->Normal * pointDistance;
        return point - scaledNormal;
    }

    bool Equals(const FPlane &Other, double epsilon=1e-6)
    {
        return FMath::equalsEpsilon(Other.Normal, Normal, epsilon) && FMath::equalsEpsilon(Other.Distance, Distance, epsilon);
    }

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

template<typename T>
struct TBox
{
public:

    /** Holds the box's minimum point. */
    TVector<T> Min;

    /** Holds the box's maximum point. */
    TVector<T> Max;

public:

    TBox() : Min(0), Max(0) { }

    TBox(const TVector<T> &InMin, const TVector<T> &InMax)
        : Min(InMin), Max(InMax) { }

    TBox TransformBy(const TTransform<T> &Transform) const
    {
        TBox NewBox;
        glm::dmat4 M = Transform.ToMatrix();

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

    ECullingResult IntersectPlane(const FPlane& plane) const noexcept
    {
        const glm::dvec3 normal = plane.Normal;

        const dvec3 Center = (Min+Max) / 2.0;

        // const glm::dvec3& boxVertex1 = HalfAxes[0] + HalfAxes[1] + HalfAxes[2];
        // const glm::dvec3& boxVertex2 = -HalfAxes[0] + HalfAxes[1] + HalfAxes[2];
        // const glm::dvec3& boxVertex3 = HalfAxes[0] - HalfAxes[1] + HalfAxes[2];
        // const glm::dvec3& boxVertex4 = HalfAxes[0] + HalfAxes[1] - HalfAxes[2];

        
        glm::dvec3 boxVertex;
        boxVertex.x = normal.x > 0 ? Max.x : Min.x;
        boxVertex.y = normal.y > 0 ? Max.y : Min.y;
        boxVertex.z = normal.z > 0 ? Max.z : Min.z;

        // plane is used as if it is its normal; the first three components are
        // assumed to be normalized
        // const double radEffective1 = glm::abs(glm::dot(normal, boxVertex));
        // const double radEffective2 = glm::abs(glm::dot(normal, boxVertex2));
        // const double radEffective3 = glm::abs(glm::dot(normal, boxVertex3));
        // const double radEffective4 = glm::abs(glm::dot(normal, boxVertex4));

        // const double radEffective = glm::max(radEffective1, glm::max(radEffective2, glm::max(radEffective3, radEffective4)));

        const double radEffective = glm::abs(glm::dot(normal, boxVertex));

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

};

using FBox = TBox<double>;

}