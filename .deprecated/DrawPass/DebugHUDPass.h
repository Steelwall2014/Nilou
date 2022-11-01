#pragma once
#include "Interface/IDrawPass.h"

namespace und {
	class DebugHUDPass : implements IDrawPass
	{
	public:
		virtual void Draw(FrameVariables &frame) override;
	};
}