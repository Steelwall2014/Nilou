#pragma once
#include "Interface/IDrawPass.h"

namespace und {
	class ShadowMappingPass :implements IDrawPass
	{
	public:
		virtual void Draw(FrameVariables &frame) override;
	};
}