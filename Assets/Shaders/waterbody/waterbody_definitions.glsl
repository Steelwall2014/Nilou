struct WaterbodyParameters
{
	float water_depth;
	float fog_density;
	float zeta;
	float gf;
	float gb;
	float hdr_exposure;
	vec3 pure_water_scattering;
	vec3 minerals_scattering;
	vec3 phytoplankton_scattering;
	
	vec3 pure_water_absorbtion;
	vec3 minerals_absorbtion;
	vec3 phytoplankton_absorbtion;
	vec3 CDOM_absorbtion;

	vec3 total_scattering;	// = pure_water_scattering + minerals_scattering + phytoplankton_scattering
	vec3 total_absorbtion;	// = pure_water_absorbtion + minerals_absorbtion + phytoplankton_absorbtion + CDOM_absorbtion
	vec3 total_extinction;	// = total_scattering + total_absorbtion
};
uniform WaterbodyParameters WATERBODY;
