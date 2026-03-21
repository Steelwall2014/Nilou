#pragma once
#include "SceneComponent.h"
// #include "ShaderParameterBlock.h"
#include "UniformBuffer.h"
#include "RenderGraph.h"
#include "atmosphere_definitions.generated.h"

namespace nilou {

    class NCLASS USkyAtmosphereComponent : public USceneComponent
    {
        GENERATED_BODY()
    public:
        USkyAtmosphereComponent();

		// The solar irradiance at the top of the atmosphere.
		FVector3f SolarIrradiance;
		// The sun's angular radius. Warning: the implementation uses approximations
		// that are valid only if this angle is smaller than 0.1 radians.
		float SunAngularRadius;
		// The distance between the planet center and the bottom of the atmosphere.
		float BottomRadius;
		// The distance between the planet center and the top of the atmosphere.
		float TopRadius;
		// The density profile of air molecules, i.e. a function from altitude to
		// dimensionless values between 0 (null density) and 1 (maximum density).
		// SKY_DECLARE_FUNCTION(shader::DensityProfile<Std140Layout>, RayleighDensity)
		shader::DensityProfile<Std140Layout> RayleighDensity;
		// The scattering coefficient of air molecules at the altitude where their
		// density is maximum (usually the bottom of the atmosphere), as a function of
		// wavelength. The scattering coefficient at altitude h is equal to
		// 'rayleigh_scattering' times 'rayleigh_density' at this altitude.
		FVector3f RayleighScattering;
		// The density profile of aerosols, i.e. a function from altitude to
		// dimensionless values between 0 (null density) and 1 (maximum density).
		shader::DensityProfile<Std140Layout> MieDensity;
		// The scattering coefficient of aerosols at the altitude where their density
		// is maximum (usually the bottom of the atmosphere), as a function of
		// wavelength. The scattering coefficient at altitude h is equal to
		// 'mie_scattering' times 'mie_density' at this altitude.
		FVector3f MieScattering;
		// The extinction coefficient of aerosols at the altitude where their density
		// is maximum (usually the bottom of the atmosphere), as a function of
		// wavelength. The extinction coefficient at altitude h is equal to
		// 'mie_extinction' times 'mie_density' at this altitude.
		FVector3f MieExtinction;
		// The asymetry parameter for the Cornette-Shanks phase function for the
		// aerosols.
		float MiePhaseFunction_g;
		// The density profile of air molecules that absorb light (e.g. ozone), i.e.
		// a function from altitude to dimensionless values between 0 (null density)
		// and 1 (maximum density).
		shader::DensityProfile<Std140Layout> AbsorptionDensity;
		// The extinction coefficient of molecules that absorb light (e.g. ozone) at
		// the altitude where their density is maximum, as a function of wavelength.
		// The extinction coefficient at altitude h is equal to
		// 'absorption_extinction' times 'absorption_density' at this altitude.
		FVector3f AbsorptionExtinction;
		// The average albedo of the ground.
		FVector3f GroundAlbedo;
		// The cosine of the maximum Sun zenith angle for which atmospheric scattering
		// must be precomputed (for maximum precision, use the smallest Sun zenith
		// angle yielding negligible sky light radiance values. For instance, for the
		// Earth case, 102 degrees is a good choice - yielding mu_s_min = -0.2).
		float Mu_s_Min;

    protected:
        //~ Begin UActorComponent Interface.
        virtual void CreateRenderState() override;
        virtual void DestroyRenderState() override;
	    //~ End UActorComponent Interface.

        class FSkyAtmosphereSceneProxy *SkyAtmosphereSceneProxy;
    };
#undef SKY_DECLARE_FUNCTION

    class FSkyAtmosphereSceneProxy
    {
    public:
	    FSkyAtmosphereSceneProxy(const USkyAtmosphereComponent* InComponent);

		inline RDGTextureView *GetTransmittanceLUT() const { return TransmittanceLUT->GetDefaultView(); }
		inline RDGTextureView *GetMultiScatteringLUT() const { return MultiScatteringLUT->GetDefaultView(); }
		inline RDGTextureView *GetSingleScatteringMieLUT() const { return SingleScatteringMieLUT->GetDefaultView(); }


	protected:
		TParameterBlockRef<shader::AtmosphereParameters> AtmosphereParamBlock;
		RDGDescriptorSetRef AtmosphereParametersDescriptorSet;
		RDGTextureRef TransmittanceLUT;
		RDGTextureRef IrradianceLUT;
		RDGTextureRef DeltaScatteringRayleighLUT;
		RDGTextureRef SingleScatteringMieLUT;
		RDGTextureRef MultiScatteringLUT;
		RDGTextureRef ScatteringDensityLUT;

		void DispatchPrecompute();

		void DispatchTransmittancePass();
		void DispatchDirectIrradiancePass();
		void DispatchScatteringPass();
		void DispatchScatteringDensityPass(int32 scattering_order);
		void DispatchIndirectIrradiancePass(int32 scattering_order);
		void DispatchMultiScatteringPass();
    };

}