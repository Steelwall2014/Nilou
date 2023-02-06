#pragma once
#include "Actor.h"
#include "Georeference.h"

namespace nilou {

	UCLASS()
    class AGeoreferenceActor : public AActor
    {
		GENERATE_CLASS_INFO()
    public:
        AGeoreferenceActor()
            : GeoreferencedToEcef(1)
            , EcefToGeoreferenced(1)
            , AbsToEcef(1)
            , EcefToAbs(1)
        {

        }

        void SetGeoreferenceOrigin(double InOriginLongitude, double InOriginLatitude, double InOriginHeight);

        void UpdateGeoreference();

    private:

        double OriginLongitude;
        double OriginLatitude;
        double OriginHeight;

        // ENU转ECEF (ENU->XYZ)
        glm::dmat4 GeoreferencedToEcef;
        // ECEF转ENU （XYZ->ENU）
        glm::dmat4 EcefToGeoreferenced;
        // 绝对坐标转 ECEF 
        glm::dmat4 AbsToEcef;
        // ECEF 转 绝对坐标
        glm::dmat4 EcefToAbs;

    };

}