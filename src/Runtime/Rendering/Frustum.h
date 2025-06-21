#pragma once

// #include <glm/glm.hpp>
#include "Common/Math/BoxSphereBounds.h"
#include "Common/Math/Transform.h"
#include "SerializeHelper.h"
#include "Common/CoreUObject/Class.h"

namespace nilou {

    struct FOrientedBoundingBox
    {  
        FOrientedBoundingBox() { }
        /**
        * @brief Constructs a new instance.
        *
        * @param center The center of the box.
        * @param halfAxes The three orthogonal half-axes of the bounding box.
        * Equivalently, the transformation matrix to rotate and scale a 2x2x2 cube
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

        FBoundingSphere TransformBy(const dmat4 &Transform) const;
    };


    // Axis Aligned
    // struct NSTRUCT FBoundingBox
    // {
    //     GENERATED_STRUCT_BODY()

    //     NPROPERTY()
    //     dvec3 Min;
    //     NPROPERTY()
    //     dvec3 Max;
    //     FBoundingBox() { Min = Max = dvec3(0); }

    //     FBoundingBox(const dvec3 &Min, const dvec3 &Max);

    //     FBoundingBox(const FOrientedBoundingBox &OBB) 
    //         : FBoundingBox(OBB.Center, OBB.HalfAxes[0], OBB.HalfAxes[1], OBB.HalfAxes[2]) { }

    //     FBoundingBox(const dvec3 &Center, const dvec3 &xDirection, const dvec3 &yDirection, const dvec3 &zDirection);

    //     FBoundingBox TransformBy(const FTransform &Transform) const;

    //     void FromBoundingSphere(const FBoundingSphere &Sphere);

    //     ECullingResult IntersectPlane(const FPlane& plane) const noexcept;
    // };

    // template<>
    // class TStaticSerializer<FBoundingBox>
    // {
    // public:
    //     static void Serialize(const FBoundingBox &Object, nlohmann::json &json, FArchiveBuffers &Buffers)
    //     {
    //         json["ClassName"] = "FBoundingBox";
    //         nlohmann::json &content = json["Content"];
    //         content["Min"].push_back(Object.Min.x);
    //         content["Min"].push_back(Object.Min.y);
    //         content["Min"].push_back(Object.Min.z);
    //         content["Max"].push_back(Object.Max.x);
    //         content["Max"].push_back(Object.Max.y);
    //         content["Max"].push_back(Object.Max.z);
    //     }
    //     static void Deserialize(FBoundingBox &Object, nlohmann::json &json, void* Buffer)
    //     {
    //         if (!SerializeHelper::CheckIsType(json, "FBoundingBox")) return;
    //         nlohmann::json &content = json["Content"];
    //         Object.Min.x = content["Min"][0].get<double>();
    //         Object.Min.y = content["Min"][1].get<double>();
    //         Object.Min.z = content["Min"][2].get<double>();
    //         Object.Max.x = content["Max"][0].get<double>();
    //         Object.Max.y = content["Max"][1].get<double>();
    //         Object.Max.z = content["Max"][2].get<double>();
    //     }
    // };

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
        bool IsBoxOutSidePlane(const FPlane &plane, const FBoxSphereBounds &AABB) const;
        bool IsBoxOutSidePlane(const FPlane &plane, const FOrientedBoundingBox &OBB) const;
        
        /**
         * @brief Calculate if the oriented bounding box is out of frustum
         */
        bool IsBoxOutSideFrustum(const FBoxSphereBounds &AABB) const;

        /**
         * @brief Calculate if the oriented bounding box is out of frustum
         */
        bool IsBoxOutSideFrustum(const FOrientedBoundingBox &OBB) const;
        bool IsBoxOutSideFrustum(const dvec3 &Center, const dmat3& HalfAxes) const;

        /**
         * @brief Calculate if the axis-aligned bounding box is out of frustum, 
         * ignoring near and far clip plane to make it faster.
         */
        bool IsBoxOutSideFrustumFast(const FBoxSphereBounds &AABB) const;

        /**
         * @brief Calculate if the oriented bounding box is out of frustum, 
         * ignoring near and far clip plane to make it faster.
         */
        bool IsBoxOutSideFrustumFast(const FOrientedBoundingBox &OBB) const;
        bool IsBoxOutSideFrustumFast(const dvec3 &Center, const dmat3& HalfAxes) const;

        bool operator==(const FViewFrustum &Other);
    private:
        bool IsOutSidePlane(const FPlane &plane, glm::dvec3 position) const;
    };

    class FConvexVolume
    {
    public:
        
	    std::vector<FPlane> Planes;

        FConvexVolume()
        { }

        FConvexVolume(const std::vector<FPlane>& InPlanes) :
            Planes(InPlanes)
        { }

        bool IntersectBox(const FBoxSphereBounds &Box);
    };

}