#pragma once
#include "PrimitiveComponent.h"

namespace nilou {

    UCLASS()
    class UFourierTransformOceanComponent : public UPrimitiveComponent
    {
        GENERATE_CLASS_INFO()
        friend class FFourierTransformOceanSceneProxy;
    public:

        UFourierTransformOceanComponent(AActor* Owner=nullptr);

        virtual FPrimitiveSceneProxy* CreateSceneProxy() override;

    protected:

        // Ocean surface size (in world unit)
        float SurfaceSize = 40960.f;

        // Wind direction, must be normalized
		vec2 WindDirection = glm::normalize(vec2(1));

        float WindSpeed = 6.5f;

        uint32 FFTPow = 9;

        float Amplitude = 0.45f * 1e-3f;

        // Corresponding to UE5 WorldAlignedTexture TextureSize pin
        float DisplacementTextureSize = 0.05 * glm::pow(2, FFTPow);

    };

}