#pragma once
#include "Interface/IDrawPass.h"
#include "RHIResources.h"

namespace und {
	class SceneObjectWaterbody;
	
	class OceanScatteringPrecomputeSubPass : ISubPass
	{
	private:
		RHITexture3DRef m_ScatteringLUT;
		RHITexture3DRef m_MultiScatteringDensityLUT;
	public:
		void Initialize(std::shared_ptr<SceneObjectWaterbody> waterbody);
		void DrawOrCompute(std::shared_ptr<SceneObjectWaterbody> waterbody);
	};
}