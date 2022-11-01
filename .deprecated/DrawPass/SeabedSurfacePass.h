#pragma once
#include "Common/DrawPass/HugeSurfacePass.h"
#include "Common/SceneObject.h"


namespace und {
	class SeabedSurfacePass : public HugeSurfacePass
	{
	private:
		RHITexture2DRef m_HeightMap;
		RHITexture2DRef m_Normal;
		RHITexture2DRef m_MaterialBaseColorMap;
		RHITexture2DRef m_MaterialNormalMap;
		RHITexture2DRef m_MaterialRoughnessMap;
		std::vector<RHITexture2DRef> m_Caustics;
		//OpenGLTextureImage3DRef m_ScatteringLUT;
	public:
		virtual int Initialize(FrameVariables &frame) override;
		virtual void Draw(FrameVariables &frame) override;
	};
}
