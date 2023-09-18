#pragma once
#include "Actor.h"
#include "Georeference.h"

namespace nilou {

    class NCLASS AGeoreferenceActor : public AActor
    {
		GENERATED_BODY()
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

        const glm::dmat4 &GetGeoreferencedToEcef() const { return GeoreferencedToEcef; }

        const glm::dmat4 &GetEcefToGeoreferenced() const { return EcefToGeoreferenced; }

        const glm::dmat4 &GetAbsToEcef() const { return AbsToEcef; }

        const glm::dmat4 &GetEcefToAbs() const { return EcefToAbs; }

    private:

        NPROPERTY()
        double OriginLongitude;
        NPROPERTY()
        double OriginLatitude;
        NPROPERTY()
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