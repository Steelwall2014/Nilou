#include "Common/SceneNode/SceneLightNode.h"
#include "SceneObjectAtmosphere.h"

namespace und {
	SceneObjectAtmosphere::SceneObjectAtmosphere(std::shared_ptr<SceneLightNode> Sun)
		: BaseSceneObject(kSceneObjectTtypeAtmosphere)
		, Sun(Sun)
	{
		if (Sun->GetSceneObjectRef()->GetType() != SceneObjectType::kSceneObjectTypeDirectionalLight)
			throw("Sun light must be directional light");
		solar_irradiance = { 1.474000, 1.850400, 1.911980 };
		sun_angular_radius = 0.004675;
		bottom_radius = 6360.000000;
		top_radius = 6420.000000;
		rayleigh_density.layers[0] = { 0.000000,0.000000,0.000000,0.000000,0.000000 };
		rayleigh_density.layers[1] = { 0.000000,1.000000,-0.125000,0.000000,0.000000 };
		rayleigh_scattering = { 0.005802,0.013558,0.033100 };	// Ã¿Ç§Ã×
		mie_density.layers[0] = { 0.000000,0.000000,0.000000,0.000000,0.000000 };
		mie_density.layers[1] = { 0.000000,1.000000,-0.833333,0.000000,0.000000 };
		mie_scattering = { 0.003996, 0.003996, 0.003996 };
		mie_extinction = { 0.004440, 0.004440, 0.004440 };
		mie_phase_function_g = 0.800000;
		absorption_density.layers[0] = { 25.000000,0.000000,0.000000,0.066667,-0.666667 };
		absorption_density.layers[1] = { 0.000000,0.000000,0.000000,-0.066667,2.666667 };
		absorption_extinction = { 0.000650, 0.001881, 0.000085 };
		ground_albedo = { 0.100000, 0.100000, 0.100000 };
		mu_s_min = -0.207912;

		//solar_irradiance = { 1.474000, 1.850400, 1.911980 };
		//sun_angular_radius = 0.004675;
		//bottom_radius = 6357.000000;
		//top_radius = 6360.000000;
		//rayleigh_density.layers[0] = { 0.000000,0.000000,0.000000,0.000000,0.000000 };
		//rayleigh_density.layers[1] = { 0.000000,1.000000,0,0.000000,0.000000 };
		//rayleigh_scattering = { 6.9788269477924505,16.308697157179797,39.816155168896 };
		//mie_density.layers[0] = { 0.000000,0.000000,0.000000,0.000000,0.000000 };
		//mie_density.layers[1] = { 0.000000,1.000000,-0.833333,0.000000,0.000000 };
		//mie_scattering = { 0.003996, 0.003996, 0.003996 };
		//mie_extinction = { 0.004440, 0.004440, 0.004440 };
		//mie_phase_function_g = 0.800000;
		//absorption_density.layers[0] = { 25.000000,0.000000,0.000000,0.066667,-0.666667 };
		//absorption_density.layers[1] = { 0.000000,0.000000,0.000000,-0.066667,2.666667 };
		//absorption_extinction = { 0.000650, 0.001881, 0.000085 };
		//ground_albedo = { 0.100000, 0.100000, 0.100000 };
		//mu_s_min = -0.207912;
	}
}
