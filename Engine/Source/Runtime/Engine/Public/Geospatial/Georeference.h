#pragma once
#include "Ellipsoid.h"

namespace nilou {

    namespace GeoTransform {

    glm::dmat4x4 EastNorthUpToFixedFrame(
        const glm::dvec3& origin, 
        const Geospatial::Ellipsoid& ellipsoid /*= Ellipsoid::WGS84*/) noexcept;

    glm::dvec3 LongitudeLatitudeHeightToEcef(
        const glm::dvec3& LongitudeLatitudeHeight, 
        const Geospatial::Ellipsoid& ellipsoid /*= Ellipsoid::WGS84*/) noexcept;

    }
}