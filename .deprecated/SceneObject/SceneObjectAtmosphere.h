#pragma once
#include "Common/BaseSceneObject.h"

namespace und {
	class SceneLightNode;
	// An atmosphere layer of width 'width', and whose density is defined as
	//   'exp_term' * exp('exp_scale' * h) + 'linear_term' * h + 'constant_term',
	// clamped to [0,1], and where h is the altitude.
	struct DensityProfileLayer {
		float width;
		float exp_term;
		float exp_scale;
		float linear_term;
		float constant_term;
	};

	// An atmosphere density profile made of several layers on top of each other
	// (from bottom to top). The width of the last layer is ignored, i.e. it always
	// extend to the top atmosphere boundary. The profile values vary between 0
	// (null density) to 1 (maximum density).
	struct DensityProfile {
		DensityProfileLayer layers[2];
	};

	class SceneObjectAtmosphere : public BaseSceneObject
	{
	public:
		std::shared_ptr<SceneLightNode> Sun;
		// The solar irradiance at the top of the atmosphere.
		glm::vec3 solar_irradiance;
		// The sun's angular radius. Warning: the implementation uses approximations
		// that are valid only if this angle is smaller than 0.1 radians.
		float sun_angular_radius;
		// The distance between the planet center and the bottom of the atmosphere.
		float bottom_radius;
		// The distance between the planet center and the top of the atmosphere.
		float top_radius;
		// The density profile of air molecules, i.e. a function from altitude to
		// dimensionless values between 0 (null density) and 1 (maximum density).
		DensityProfile rayleigh_density;
		// The scattering coefficient of air molecules at the altitude where their
		// density is maximum (usually the bottom of the atmosphere), as a function of
		// wavelength. The scattering coefficient at altitude h is equal to
		// 'rayleigh_scattering' times 'rayleigh_density' at this altitude.
		glm::vec3 rayleigh_scattering;
		// The density profile of aerosols, i.e. a function from altitude to
		// dimensionless values between 0 (null density) and 1 (maximum density).
		DensityProfile mie_density;
		// The scattering coefficient of aerosols at the altitude where their density
		// is maximum (usually the bottom of the atmosphere), as a function of
		// wavelength. The scattering coefficient at altitude h is equal to
		// 'mie_scattering' times 'mie_density' at this altitude.
		glm::vec3 mie_scattering;
		// The extinction coefficient of aerosols at the altitude where their density
		// is maximum (usually the bottom of the atmosphere), as a function of
		// wavelength. The extinction coefficient at altitude h is equal to
		// 'mie_extinction' times 'mie_density' at this altitude.
		glm::vec3 mie_extinction;
		// The asymetry parameter for the Cornette-Shanks phase function for the
		// aerosols.
		float mie_phase_function_g;
		// The density profile of air molecules that absorb light (e.g. ozone), i.e.
		// a function from altitude to dimensionless values between 0 (null density)
		// and 1 (maximum density).
		DensityProfile absorption_density;
		// The extinction coefficient of molecules that absorb light (e.g. ozone) at
		// the altitude where their density is maximum, as a function of wavelength.
		// The extinction coefficient at altitude h is equal to
		// 'absorption_extinction' times 'absorption_density' at this altitude.
		glm::vec3 absorption_extinction;
		// The average albedo of the ground.
		glm::vec3 ground_albedo;
		// The cosine of the maximum Sun zenith angle for which atmospheric scattering
		// must be precomputed (for maximum precision, use the smallest Sun zenith
		// angle yielding negligible sky light radiance values. For instance, for the
		// Earth case, 102 degrees is a good choice - yielding mu_s_min = -0.2).
		float mu_s_min;

		SceneObjectAtmosphere(std::shared_ptr<SceneLightNode> Sun);
	};
}