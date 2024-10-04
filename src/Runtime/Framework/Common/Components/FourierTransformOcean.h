#pragma once
#include "PrimitiveComponent.h"
#include "Material.h"

namespace nilou {

    class UTexture2D;

    BEGIN_UNIFORM_BUFFER_STRUCT(FOceanFastFourierTransformParameters)
        SHADER_PARAMETER(vec2, WindDirection)
        SHADER_PARAMETER(uint32, N)
        SHADER_PARAMETER(float, WindSpeed)
        SHADER_PARAMETER(float, Amplitude)
        SHADER_PARAMETER(float, Time)
    END_UNIFORM_BUFFER_STRUCT()

    BEGIN_UNIFORM_BUFFER_STRUCT(FOceanFFTButterflyBlock)
        SHADER_PARAMETER(uint32, Ns)
    END_UNIFORM_BUFFER_STRUCT()

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

        void SetWindDirection(const vec2& InWindDirection) 
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
		vec2 WindDirection = glm::normalize(vec2(1));

        float WindSpeed = 6.5f;

        uint32 FFTPow = 9;

        float Amplitude = 0.45f * 1e-3f;

        // Corresponding to UE5 WorldAlignedTexture TextureSize pin
        float DisplacementTextureSize = 0.05 * glm::pow(2, FFTPow);

        float InitialTime;

        std::shared_ptr<UTexture2D> GaussianRandomTexture;

        UTexture2D* PerlinNoise;

        TRDGUniformBufferRef<FOceanFastFourierTransformParameters> FFTParameters;

    };

}