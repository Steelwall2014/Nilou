#pragma once
#include <iostream>
#include <memory>
#include <vector>
#include <glad/glad.h>

#include "RHIResources.h"
#include "Common/Log.h"

namespace nilou {
	// class OpenGLShader;
	// class OpenGLVertexShader;
	// class OpenGLPixelShader;
	// class OpenGLComputeShader;
	// class OpenGLLinkedProgram;
	// typedef std::shared_ptr<OpenGLShader> OpenGLShaderRef;
	// typedef std::shared_ptr<OpenGLVertexShader> OpenGLVertexShaderRef;
	// typedef std::shared_ptr<OpenGLPixelShader> OpenGLPixelShaderRef;
	// typedef std::shared_ptr<OpenGLComputeShader> OpenGLComputeShaderRef;
	// typedef std::shared_ptr<OpenGLLinkedProgram> OpenGLLinkedProgramRef;

	template<typename BaseType, GLenum ShaderType>
	class TOpenGLShader : public BaseType
	{
		static_assert(
			ShaderType == GL_VERTEX_SHADER || 
			ShaderType == GL_FRAGMENT_SHADER || 
			ShaderType == GL_COMPUTE_SHADER , 
			"TOpenGLShader template parameter error: Only GL_VERTEX_SHADER, GL_FRAGMENT_SHADER or GL_COMPUTE_SHADER are supported");
	public:
		virtual bool Success() override
		{
			int success;
			char infoLog[512];
			glGetShaderiv(Resource, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(Resource, 512, NULL, infoLog);
				NILOU_LOG(Error, "Vertex/Pixel/Compute shader compilation failed\n" + std::string(infoLog))
				return false;
			}
			return true;
		}
		virtual ~TOpenGLShader() { glDeleteShader(Resource); }
		TOpenGLShader(const char *shader_code)
		{
			Resource = glCreateShader(ShaderType);
			glShaderSource(Resource, 1, &shader_code, 0);
			glCompileShader(Resource);
		}
		GLuint Resource;
		friend class OpenGLLinkedProgram;
	private:
		TOpenGLShader() : Resource(0) {};
	};

	using OpenGLVertexShader = TOpenGLShader<RHIVertexShader, GL_VERTEX_SHADER>;
	using OpenGLPixelShader = TOpenGLShader<RHIPixelShader, GL_FRAGMENT_SHADER>;
	using OpenGLComputeShader = TOpenGLShader<RHIComputeShader, GL_COMPUTE_SHADER>;

	using OpenGLVertexShaderRef = std::shared_ptr<OpenGLVertexShader>;
	using OpenGLPixelShaderRef = std::shared_ptr<OpenGLPixelShader>;
	using OpenGLComputeShaderRef = std::shared_ptr<OpenGLComputeShader>;
	
	class OpenGLLinkedProgram// : public RHILinkedProgram
	{
	public:
		bool Success()
		{
			int success;
			char infoLog[512];
			glGetProgramiv(Resource, GL_LINK_STATUS, &success);
			if (!success)
			{
				glGetProgramInfoLog(Resource, 512, NULL, infoLog);
				std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
				return false;
			}
			return true;
		}
		OpenGLLinkedProgram(OpenGLComputeShader *ComputeShader);
		OpenGLLinkedProgram(OpenGLVertexShader *VertexShader, OpenGLPixelShader *PixelShader);
		virtual ~OpenGLLinkedProgram();

		OpenGLVertexShader	*VertexShader;
		OpenGLPixelShader 	*PixelShader;
		OpenGLComputeShader 	*ComputeShader;
		GLuint Resource;
		friend class FOpenGLDynamicRHI;
	};
	using OpenGLLinkedProgramRef = std::shared_ptr<OpenGLLinkedProgram>;
	// class OpenGLVertexShader : public OpenGLShader
	// {
	// public:
	// 	OpenGLVertexShader(const char *shader_code) : OpenGLShader(shader_code, GL_VERTEX_SHADER) {};
	// };

	// class OpenGLPixelShader : public OpenGLShader
	// {
	// public:
	// 	OpenGLPixelShader(const char *shader_code) : OpenGLShader(shader_code, GL_FRAGMENT_SHADER) {};
	// };

	// class OpenGLComputeShader : public OpenGLShader
	// {
	// public:
	// 	OpenGLComputeShader(const char *shader_code) : OpenGLShader(shader_code, GL_COMPUTE_SHADER) {};
	// };

}