#pragma once
#include "PrimitiveComponent.h"
#include "Materials/Material.h"
#include "RenderGraphParameterBlock.h"
#include "OceanFastFourierTransformParameters.generated.h"

namespace nilou {

    class UTexture2D;

    class NCLASS UFourierTransformOceanComponent : public UActorComponent
    {
        GENERATED_BODY()
        friend class FFourierTransformOceanSceneProxy;
    public:

        UFourierTransformOceanComponent();

        virtual void TickComponent(double DeltaTime) override;

        void UpdateHeightField();

        std::shared_ptr<UTexture2D> DisplaceTexture;

        std::shared_ptr<UTexture2D> NormalTexture;

        std::shared_ptr<UTexture2D> FoamTexture;

        void SetMaterial();

        void SetWindDirection(const FVector2f& InWindDirection) 
        { 
            WindDirection = glm::normalize(InWindDirection);
        }

        void SetWindSpeed(float InWindSpeed) 
        { 
            WindSpeed = InWindSpeed;
        }

        void SetFFTPow(uint32 InFFTPow) 
        { 
            FFTPow = InFFTPow;
        }

        void SetAmplitude(float InAmplitude) 
        { 
            Amplitude = InAmplitude;
        }

    protected:

        // Wind direction, must be normalized
		FVector2f WindDirection = glm::normalize(FVector2f(1));

        float WindSpeed = 6.5f;

        uint32 FFTPow = 9;

        float Amplitude = 0.45f * 1e-3f;

        // Corresponding to UE5 WorldAlignedTexture TextureSize pin
        float DisplacementTextureSize = 0.05 * glm::pow(2, FFTPow);

        float InitialTime;

        std::shared_ptr<UTexture2D> GaussianRandomTexture;

        UTexture2D* PerlinNoise;

        TParameterBlockRef<shader::FOceanFastFourierTransformParameters> FFTParameters;

    };

}