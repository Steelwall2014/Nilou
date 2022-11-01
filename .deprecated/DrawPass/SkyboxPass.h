#pragma once
#include "Interface/IDrawPass.h"
#include "OpenGL/OpenGLVertexArrayObject.h"
#include "OpenGL/OpenGLTexture.h"
#include "Common/SceneObject.h"
#include "RHIResources.h"

namespace und {
	class SkyboxPass : implements IDrawPass
	{
	private:
		RHIVertexArrayObjectRef m_SkyboxVAO;
		RHITexture2DRef m_TransmittanceLUT;
		RHITexture2DRef m_IrradianceLUT;
		RHITexture3DRef m_DeltaScatteringRayleighLUT;
		RHITexture3DRef m_SingleScatteringMieLUT;
		RHITexture3DRef m_MultiScatteringLUT;
		RHITexture2DRef m_SkyMap;
		std::shared_ptr<SceneObjectAtmosphere> m_Atmosphere; 
		void UpdateSkyMap(FrameVariables &frame);
	public:
		virtual int Initialize(FrameVariables &frame) override;
		virtual void Draw(FrameVariables &frame) override;
	};
}