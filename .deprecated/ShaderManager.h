#pragma once
#include <map>
#include <string>
#include "Interface/IRuntimeModule.h"
#include "RHIResources.h"

namespace und {
	class ShaderManager : implements IRuntimeModule
	{
	private:
		std::map<std::string, und::RHILinkedProgramRef> m_Shaders;	// string是shader的名称
	public:
		bool LoadShader(const std::string vert_filepath, const std::string frag_filepath, const char *shader_name);
		bool LoadShader(const std::string comp_filepath, const char *shader_name);
		bool LoadShader(const char *vert_filepath, const char *frag_filepath, const char *shader_name);
		bool LoadShader(const char *comp_filepath, const char *shader_name);
		und::RHILinkedProgramRef GetShaderByName(const char *shader_name);
		virtual void Tick(double) {}
		virtual int Initialize();
		virtual void Finalize() {}
	};

	extern ShaderManager *g_pShaderManager;
}