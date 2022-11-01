#pragma once
#include <map>
#include <string>

#include "RHIResources.h"

namespace und {
	class TextureManager
	{
	private:
		std::map<std::string, RHITextureRef> GlobalTextures;
	public:
		void AddGlobalTexture(const std::string &name, RHITextureRef texture, bool overlap = false);
		void RemoveGlobalTexture(const std::string &name);
		RHITextureRef GetGlobalTexture(const std::string &name);
	};
	extern TextureManager *g_pTextureManager;
}