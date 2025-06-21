#pragma once

#include "Maths.h"
#include "Box.h"

namespace nilou {
    
template<typename T, typename TExtent>
struct TBoxSphereBounds
{

    /** Holds the origin of the bounding box and sphere. */
    TVector<T>	Origin;

    /** Holds the extent of the bounding box. */
    TVector<TExtent> BoxExtent;

    /** Holds the radius of the bounding sphere. */
    TExtent SphereRadius;

    TBoxSphereBounds() : Origin(0), BoxExtent(0), SphereRadius(0) { }

    TBoxSphereBounds(const TVector<T> &InOrigin, const TVector<TExtent> &InBoxExtent, TExtent InSphereRadius)
        : Origin(InOrigin), BoxExtent(InBoxExtent), SphereRadius(InSphereRadius) { } 

    TBoxSphereBounds(const TBox<T> &InBox)
        : Origin(InBox.Min + InBox.Max * 0.5)
        , BoxExtent((InBox.Max - InBox.Min) * 0.5)
        , SphereRadius(glm::length(InBox.Max - InBox.Min) * 0.5) { }

    TBox<T> GetBox() const 
    { 
        return TBox<T>(Origin - BoxExtent, Origin + BoxExtent); 
    }

};

using FBoxSphereBounds = TBoxSphereBounds<double, double>;

}