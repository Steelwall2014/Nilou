#pragma once
#include "Interface/IDrawPass.h"

#include "RHIResources.h"

namespace und {
	class DeferredRenderPass : implements IDrawPass
	{
	private:
		RHIVertexArrayObjectRef m_ScreenVAO;
		RHIBufferRef m_ScreenVertices;
		RHIBufferRef m_ScreenUVs;
	public:
		virtual int Initialize(FrameVariables &frame) override;
		virtual void Draw(FrameVariables &frame) override;
	};
}