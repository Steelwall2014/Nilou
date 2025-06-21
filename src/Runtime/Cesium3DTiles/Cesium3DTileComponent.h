#pragma once
#if NILOU_ENABLE_3DTILES

#include "Common/Components/PrimitiveComponent.h"
#include "Cesium3DTilesetSelection.h"

namespace nilou {
    

    class NCLASS UCesium3DTileComponent : public UPrimitiveComponent
    {

        GENERATED_BODY()
    public:

        UCesium3DTileComponent();

        virtual void OnRegister() override;

        virtual FPrimitiveSceneProxy* CreateSceneProxy() override;

        /** Calculate the bounds of the component. Default behavior is a bounding box/sphere of zero size. */
        virtual FBoundingBox CalcBounds(const FTransform& LocalToWorld) const;

        Cesium3DTilesetSelection::Cesium3DTile* Tile;

        std::shared_ptr<GLTFParseResult> Gltf;

        glm::dmat4 ModelToLocal = glm::dmat4(1);

    };


}
#endif