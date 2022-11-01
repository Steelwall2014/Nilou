#pragma once
#include "Common/BaseActor.h"
namespace und {
	class Command
	{
	public:
		virtual void execute(BaseActor *actor) = 0;
	};
}
