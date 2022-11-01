#pragma once
#include "Common/BaseSceneObject.h"

namespace und {
	class SceneObjectWaterbody : public BaseSceneObject
	{
	public:
		float water_depth;		// รื
		float fog_density;
		float zeta;
		float gf;
		float gb;
		float hdr_exposure;
		float C;
		float absorbtion_d_400nm;
		float absorbtion_y_440nm;
		glm::vec3 pure_water_scattering;
		glm::vec3 minerals_scattering;
		glm::vec3 phytoplankton_scattering;

		glm::vec3 pure_water_absorbtion;
		glm::vec3 minerals_absorbtion;
		glm::vec3 phytoplankton_absorbtion;
		glm::vec3 CDOM_absorbtion;

		glm::vec3 total_scattering;	// = pure_water_scattering + minerals_scattering + phytoplankton_scattering
		glm::vec3 total_absorbtion;	// = pure_water_absorbtion + minerals_absorbtion + phytoplankton_absorbtion + CDOM_absorbtion
		glm::vec3 total_extinction;	// = total_scattering + total_absorbtion

		SceneObjectWaterbody();
		void UpdateWaterParams();
	};
}