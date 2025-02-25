#pragma once
#include "SceneComponent.h"

namespace nilou {

	// An atmosphere layer of width 'width', and whose density is defined as
	//   'exp_term' * exp('exp_scale' * h) + 'linear_term' * h + 'constant_term',
	// clamped to [0,1], and where h is the altitude.
	struct DensityProfileLayer {
		float width;
		float exp_term;
		float exp_scale;
		float linear_term;
		float constant_term;
        // DensityProfileLayer() = default;
        DensityProfileLayer &operator=(const DensityProfileLayer &) = default;
        bool operator==(const DensityProfileLayer &Other) const = default; 
        bool operator!=(const DensityProfileLayer &Other) const = default; 
	};

	// An atmosphere density profile made of several layers on top of each other
	// (from bottom to top). The width of the last layer is ignored, i.e. it always
	// extend to the top atmosphere boundary. The profile values vary between 0
	// (null density) to 1 (maximum density).
	struct DensityProfile {
		DensityProfileLayer layers[2];
        // DensityProfile() = default;
        DensityProfile(const DensityProfileLayer &layer1, const DensityProfileLayer &layer2)
        {
            layers[0] = layer1;
            layers[1] = layer2;
        }
        DensityProfile &operator=(const DensityProfile &Other) = default;
        bool operator==(const DensityProfile &Other) const = default; 
        bool operator!=(const DensityProfile &Other) const = default; 
	};

#define SKY_DECLARE_FUNCTION(MemberType, MemberName) \
protected: \
    MemberType MemberName; \
public: \
    inline void Set##MemberName(const MemberType &NewValue) \
    { \
        if (MemberName != NewValue) \
        { \
            MemberName = NewValue; \
            MarkRenderStateDirty(); \
        } \
    } \
    inline MemberType Get##MemberName() const \
    { \
        return MemberName; \
    }

    class NCLASS USkyAtmosphereComponent : public USceneComponent
    {
        GENERATED_BODY()
    public:
        USkyAtmosphereComponent();

		// The solar irradiance at the top of the atmosphere.
		SKY_DECLARE_FUNCTION(vec3, SolarIrradiance)
		// The sun's angular radius. Warning: the implementation uses approximations
		// that are valid only if this angle is smaller than 0.1 radians.
		SKY_DECLARE_FUNCTION(float, SunAngularRadius)
		// The distance between the planet center and the bottom of the atmosphere.
		SKY_DECLARE_FUNCTION(float, BottomRadius)
		// The distance between the planet center and the top of the atmosphere.
		SKY_DECLARE_FUNCTION(float, TopRadius)
		// The density profile of air molecules, i.e. a function from altitude to
		// dimensionless values between 0 (null density) and 1 (maximum density).
		SKY_DECLARE_FUNCTION(DensityProfile, RayleighDensity)
		// The scattering coefficient of air molecules at the altitude where their
		// density is maximum (usually the bottom of the atmosphere), as a function of
		// wavelength. The scattering coefficient at altitude h is equal to
		// 'rayleigh_scattering' times 'rayleigh_density' at this altitude.
		SKY_DECLARE_FUNCTION(vec3, RayleighScattering)
		// The density profile of aerosols, i.e. a function from altitude to
		// dimensionless values between 0 (null density) and 1 (maximum density).
		SKY_DECLARE_FUNCTION(DensityProfile, MieDensity)
		// The scattering coefficient of aerosols at the altitude where their density
		// is maximum (usually the bottom of the atmosphere), as a function of
		// wavelength. The scattering coefficient at altitude h is equal to
		// 'mie_scattering' times 'mie_density' at this altitude.
		SKY_DECLARE_FUNCTION(vec3, MieScattering)
		// The extinction coefficient of aerosols at the altitude where their density
		// is maximum (usually the bottom of the atmosphere), as a function of
		// wavelength. The extinction coefficient at altitude h is equal to
		// 'mie_extinction' times 'mie_density' at this altitude.
		SKY_DECLARE_FUNCTION(vec3, MieExtinction)
		// The asymetry parameter for the Cornette-Shanks phase function for the
		// aerosols.
		SKY_DECLARE_FUNCTION(float, MiePhaseFunction_g)
		// The density profile of air molecules that absorb light (e.g. ozone), i.e.
		// a function from altitude to dimensionless values between 0 (null density)
		// and 1 (maximum density).
		SKY_DECLARE_FUNCTION(DensityProfile, AbsorptionDensity)
		// The extinction coefficient of molecules that absorb light (e.g. ozone) at
		// the altitude where their density is maximum, as a function of wavelength.
		// The extinction coefficient at altitude h is equal to
		// 'absorption_extinction' times 'absorption_density' at this altitude.
		SKY_DECLARE_FUNCTION(vec3, AbsorptionExtinction)
		// The average albedo of the ground.
		SKY_DECLARE_FUNCTION(vec3, GroundAlbedo)
		// The cosine of the maximum Sun zenith angle for which atmospheric scattering
		// must be precomputed (for maximum precision, use the smallest Sun zenith
		// angle yielding negligible sky light radiance values. For instance, for the
		// Earth case, 102 degrees is a good choice - yielding mu_s_min = -0.2).
		SKY_DECLARE_FUNCTION(float, Mu_s_Min)

    protected:
        //~ Begin UActorComponent Interface.
        virtual void CreateRenderState() override;
        virtual void DestroyRenderState() override;
	    //~ End UActorComponent Interface.

        class FSkyAtmosphereSceneProxy *SkyAtmosphereSceneProxy;
    };
#undef SKY_DECLARE_FUNCTION


	BEGIN_UNIFORM_BUFFER_STRUCT(ShaderDensityProfileLayer)
		SHADER_PARAMETER(float, width);
		SHADER_PARAMETER(float, exp_term);
		SHADER_PARAMETER(float, exp_scale);
		SHADER_PARAMETER(float, linear_term);
		SHADER_PARAMETER(float, constant_term);
	END_UNIFORM_BUFFER_STRUCT()

	BEGIN_UNIFORM_BUFFER_STRUCT(ShaderDensityProfile)
		SHADER_PARAMETER_ARRAY(ShaderDensityProfileLayer, 2, layers);
	END_UNIFORM_BUFFER_STRUCT()

	BEGIN_UNIFORM_BUFFER_STRUCT(ShaderAtmosphereParameters)
		SHADER_PARAMETER(vec3, SolarIrradiance);
		SHADER_PARAMETER(float, SunAngularRadius)
		SHADER_PARAMETER(float, BottomRadius)
		SHADER_PARAMETER(float, TopRadius)
		SHADER_PARAMETER(ShaderDensityProfile, RayleighDensity)
		SHADER_PARAMETER(vec3, RayleighScattering)
		SHADER_PARAMETER(ShaderDensityProfile, MieDensity)
		SHADER_PARAMETER(vec3, MieScattering)
		SHADER_PARAMETER(vec3, MieExtinction)
		SHADER_PARAMETER(float, MiePhaseFunction_g)
		SHADER_PARAMETER(ShaderDensityProfile, AbsorptionDensity)
		SHADER_PARAMETER(vec3, AbsorptionExtinction)
		SHADER_PARAMETER(vec3, GroundAlbedo)
		SHADER_PARAMETER(float, Mu_s_Min)
	END_UNIFORM_BUFFER_STRUCT()

	BEGIN_UNIFORM_BUFFER_STRUCT(ShaderAtmosphereParametersBlock)
		SHADER_PARAMETER(ShaderAtmosphereParameters, ATMOSPHERE);
	END_UNIFORM_BUFFER_STRUCT()

	BEGIN_UNIFORM_BUFFER_STRUCT(ScatteringOrderBlock)
		SHADER_PARAMETER(int, ScatteringOrder);
	END_UNIFORM_BUFFER_STRUCT()

	
#define SKY_PROXY_DECLARE_FUNCTION(MemberType, MemberName) \
public: \
    inline void Set##MemberName(const MemberType &NewValue) \
    { \
		AtmosphereParameters->GetData().ATMOSPHERE.MemberName = NewValue; \
    } \
    inline MemberType Get##MemberName() const \
    { \
        return AtmosphereParameters->GetData().ATMOSPHERE.MemberName; \
    }

    class FSkyAtmosphereSceneProxy
    {
    public:
	    FSkyAtmosphereSceneProxy(const USkyAtmosphereComponent* InComponent);

		SKY_PROXY_DECLARE_FUNCTION(vec3, SolarIrradiance)
		SKY_PROXY_DECLARE_FUNCTION(float, SunAngularRadius)
		SKY_PROXY_DECLARE_FUNCTION(float, BottomRadius)
		SKY_PROXY_DECLARE_FUNCTION(float, TopRadius)
		SKY_PROXY_DECLARE_FUNCTION(ShaderDensityProfile, RayleighDensity)
		SKY_PROXY_DECLARE_FUNCTION(vec3, RayleighScattering)
		SKY_PROXY_DECLARE_FUNCTION(ShaderDensityProfile, MieDensity)
		SKY_PROXY_DECLARE_FUNCTION(vec3, MieScattering)
		SKY_PROXY_DECLARE_FUNCTION(vec3, MieExtinction)
		SKY_PROXY_DECLARE_FUNCTION(float, MiePhaseFunction_g)
		SKY_PROXY_DECLARE_FUNCTION(ShaderDensityProfile, AbsorptionDensity)
		SKY_PROXY_DECLARE_FUNCTION(vec3, AbsorptionExtinction)
		SKY_PROXY_DECLARE_FUNCTION(vec3, GroundAlbedo)
		SKY_PROXY_DECLARE_FUNCTION(float, Mu_s_Min)

		inline RDGTextureView *GetTransmittanceLUT() const { return TransmittanceLUT->GetDefaultView(); }
		inline RDGTextureView *GetMultiScatteringLUT() const { return MultiScatteringLUT->GetDefaultView(); }
		inline RDGTextureView *GetSingleScatteringMieLUT() const { return SingleScatteringMieLUT->GetDefaultView(); }
		inline RDGBuffer *GetAtmosphereParametersBlock() const { return AtmosphereParameters; }


	protected:
		TRDGUniformBufferRef<ShaderAtmosphereParametersBlock> AtmosphereParameters;
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
#undef SKY_PROXY_DECLARE_FUNCTION

}