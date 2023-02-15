#include "GeoreferenceActor.h"

namespace nilou {

    void AGeoreferenceActor::SetGeoreferenceOrigin(double InOriginLongitude, double InOriginLatitude, double InOriginHeight)
    {
        OriginLongitude = InOriginLongitude;
        OriginLatitude = InOriginLatitude;
        OriginHeight = InOriginHeight;

        UpdateGeoreference();
    }

	dmat4 affineInverse(dmat4 const& m)
	{
		dmat3 const Inv(inverse(dmat3(m)));

		return dmat4(
			dvec4(Inv[0], 0),
			dvec4(Inv[1], 0),
			dvec4(Inv[2], 0),
			dvec4(-Inv * dvec3(m[3]), 1));
	}

    void AGeoreferenceActor::UpdateGeoreference()
    {
        const Geospatial::Ellipsoid& ellipsoid = Geospatial::Ellipsoid::WGS84;
        glm::dvec3 center = GeoTransform::LongitudeLatitudeHeightToEcef(glm::dvec3(OriginLongitude, OriginLatitude, OriginHeight), ellipsoid);
        this->GeoreferencedToEcef = GeoTransform::EastNorthUpToFixedFrame(center, ellipsoid);
        this->EcefToGeoreferenced = affineInverse(this->GeoreferencedToEcef);
        glm::dmat4 unrealToOrFromCesium = glm::dmat4x4(
                                            glm::dvec4(1.0, 0.0, 0.0, 0.0),
                                            glm::dvec4(0.0, -1.0, 0.0, 0.0),
                                            glm::dvec4(0.0, 0.0, 1.0, 0.0),
                                            glm::dvec4(0.0, 0.0, 0.0, 1.0));
        this->AbsToEcef = this->GeoreferencedToEcef * unrealToOrFromCesium;
        this->EcefToAbs = unrealToOrFromCesium * this->EcefToGeoreferenced;
    }

}