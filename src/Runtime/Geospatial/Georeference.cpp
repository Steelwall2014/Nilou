#include "Common/Math/Maths.h"

#include "Georeference.h"

namespace nilou {

    namespace GeoTransform {

    glm::dmat4x4 EastNorthUpToFixedFrame(
        const glm::dvec3& origin, 
        const Geospatial::Ellipsoid& ellipsoid /*= Ellipsoid::WGS84*/) noexcept
    {
        if (FMath::equalsEpsilon(origin, glm::dvec3(0.0), 1e-14)) {
            // If x, y, and z are zero, use the degenerate local frame, which is a
            // special case
            return glm::dmat4x4(
                glm::dvec4(0.0, 1.0, 0.0, 0.0),
                glm::dvec4(-1.0, 0.0, 0.0, 0.0),
                glm::dvec4(0.0, 0.0, 1.0, 0.0),
                glm::dvec4(origin, 1.0));
        }
        if (FMath::equalsEpsilon(origin.x, 0.0, 1e-14) &&
            FMath::equalsEpsilon(origin.y, 0.0, 1e-14)) {
            // If x and y are zero, assume origin is at a pole, which is a special case.
            const double sign = glm::sign(origin.z);
            return glm::dmat4x4(
                glm::dvec4(0.0, 1.0, 0.0, 0.0),
                glm::dvec4(-1.0 * sign, 0.0, 0.0, 0.0),
                glm::dvec4(0.0, 0.0, 1.0 * sign, 0.0),
                glm::dvec4(origin, 1.0));
        }

        const glm::dvec3 up = ellipsoid.geodeticSurfaceNormal(origin);
        const glm::dvec3 east = glm::normalize(glm::dvec3(-origin.y, origin.x, 0.0));
        const glm::dvec3 north = glm::cross(up, east);

        return glm::dmat4x4(
            glm::dvec4(east, 0.0),
            glm::dvec4(north, 0.0),
            glm::dvec4(up, 0.0),
            glm::dvec4(origin, 1.0));
    }

    glm::dvec3 LongitudeLatitudeHeightToEcef(
        const glm::dvec3& LongitudeLatitudeHeight, 
        const Geospatial::Ellipsoid& ellipsoid /*= Ellipsoid::WGS84*/) noexcept
    {
        return ellipsoid.cartographicToCartesian(Geospatial::Cartographic::fromDegrees(
            LongitudeLatitudeHeight.x, LongitudeLatitudeHeight.y, LongitudeLatitudeHeight.z));
    }

    }
}