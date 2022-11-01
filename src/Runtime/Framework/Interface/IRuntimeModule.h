#pragma once
#include "Interface.h"

namespace nilou {
	Interface IRuntimeModule
	{
	public:
		virtual ~IRuntimeModule() {}
		virtual int StartupModule() = 0;		
		virtual void ShutdownModule() = 0;					
	};
}
