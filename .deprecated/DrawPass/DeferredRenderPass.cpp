#include <assert.h>
#include <memory>
#include "Common/SceneManager.h"
#include "Common/ShaderManager.h"
#include "Common/TextureManager.h"
#include "DynamicRHI.h"
#include "OpenGL/OpenGLUtils.h"
#include "Common/GfxStructures.h"
#include "DeferredRenderPass.h"

namespace und {
	int DeferredRenderPass::Initialize(FrameVariables &frame)
	{
		float uv[] = {
			0.0f, 1.0f,
			0.0f, 0.0f,
			1.0f, 1.0f,
			1.0f, 0.0f
		};
		float vertices[] = {
			-1.0f, 1.0f, 0.0f,
			-1.0f, -1.0f, 0.0f,
			1.0f, 1.0f, 0.0f,
			1.0f, -1.0f, 0.0f
		};
		m_ScreenVAO = _GM->RHICreateVertexArrayObject(EPrimitiveMode::PM_Triangle_Strip);
		
        auto VBO = _GM->RHICreateVertexAttribBuffer(0, sizeof(vertices), vertices, EVertexElementTypeFlags::VET_Float | EVertexElementTypeFlags::VET_3, EBufferUsageFlags::VertexBuffer | EBufferUsageFlags::Static);
        auto UV = _GM->RHICreateVertexAttribBuffer(1, sizeof(uv), uv, EVertexElementTypeFlags::VET_Float | EVertexElementTypeFlags::VET_2, EBufferUsageFlags::VertexBuffer | EBufferUsageFlags::Static);
        m_ScreenVAO->AddVertexAttribBuffer(VBO.first, VBO.second);
        m_ScreenVAO->AddVertexAttribBuffer(UV.first, UV.second);

		// m_ScreenVertices = _GM->RHICreateBuffer(3*sizeof(float), sizeof(vertices), EBufferUsageFlags::VertexBuffer | EBufferUsageFlags::Static, vertices);
		// m_ScreenUVs = _GM->RHICreateBuffer(2*sizeof(float), sizeof(vertices), EBufferUsageFlags::VertexBuffer | EBufferUsageFlags::Static, vertices);
		// m_ScreenVAO->AddVertexAttribBuffer(
		// 	RHIShaderResourceView(0, 3*sizeof(float), 0, EVertexElementTypeFlags::VET_Float | EVertexElementTypeFlags::VET_3), m_ScreenVertices);
		// m_ScreenVAO->AddVertexAttribBuffer(
		// 	RHIShaderResourceView(1, 2*sizeof(float), 0, EVertexElementTypeFlags::VET_Float | EVertexElementTypeFlags::VET_2), m_ScreenUVs);
		_GM->RHIInitializeVertexArrayObject(m_ScreenVAO);
		return 0;
	}
	void DeferredRenderPass::Draw(FrameVariables &frame)
	{
		_GM->RHIUseShaderProgram(g_pShaderManager->GetShaderByName("Deferred Rendering"));
		auto atmosphere = g_pSceneManager->GetScene()->Atmosphere;
		auto ocean_surface = g_pSceneManager->GetScene()->OceanSurface;
		auto waterbody = g_pSceneManager->GetScene()->Waterbody;
		SetAtmosphereParameters(atmosphere);
		SetWaterbodyParameters(waterbody);

		glm::vec3 camera_ray[4];
		float halfHeight = frame.frameContext.cameraNearClip * tan(glm::radians(frame.frameContext.cameraFOVy / 2));
		glm::vec3 toTop = frame.frameContext.cameraUp * halfHeight;
		glm::vec3 toRight = frame.frameContext.cameraRight * halfHeight * frame.frameContext.cameraAspect;
		glm::vec3 &forward = frame.frameContext.cameraForward;
		float nearClip = frame.frameContext.cameraNearClip;
		camera_ray[0] = forward * nearClip + toTop - toRight;	// 左上
		camera_ray[1] = forward * nearClip - toTop - toRight;	// 左下
		camera_ray[2] = forward * nearClip + toTop + toRight;	// 右上
		camera_ray[3] = forward * nearClip - toTop + toRight;	// 右下

		char uniformName[16];
		for (int i = 0; i < 4; i++)
		{
			sprintf(uniformName, "CameraRay[%d]", i);
			_GM->RHISetShaderParameter(uniformName, camera_ray[i]);
		}
		_GM->RHISetShaderParameter("cameraPos", frame.frameContext.cameraPosition);
		_GM->RHISetShaderParameter("SunLightDir", frame.frameContext.lights[0].lightDirection);
		_GM->RHISetShaderParameter("cameraNearClip", frame.frameContext.cameraNearClip);
		_GM->RHISetShaderParameter("cameraFarClip", frame.frameContext.cameraFarClip);
		_GM->RHISetShaderParameter("OceanSurfaceHeightMapMeterSize", ocean_surface->HeightMapMeterSize);
		
		RHIFramebufferRef frame_buffer = frame.frameContext.renderTarget;
		assert(frame_buffer != nullptr);
		assert(frame_buffer->HasColorAttachment());
		assert(frame_buffer->HasDepthAttachment());
		RHIDepthStencilState state;
		state.EnableDepthTest = false;
		_GM->RHISetDepthStencilState(state);
		RHITexture2DRef ColorBuffer = std::dynamic_pointer_cast<RHITexture2D>(frame_buffer->Attachments[EFramebufferAttachment::FA_Color_Attachment0]);
		RHITexture2DRef DepthBuffer = std::dynamic_pointer_cast<RHITexture2D>(frame_buffer->Attachments[EFramebufferAttachment::FA_Depth_Attachment]);
		RHITextureParams params;
		params.Mag_Filter = ETextureFilters::TF_Nearest;
		params.Min_Filter = ETextureFilters::TF_Nearest;
		_GM->RHIBindTexture("ColorBuffer", ColorBuffer, params);
		_GM->RHIBindTexture("DepthBuffer", DepthBuffer, params);
		RHITextureRef OceanSurfaceHeightMap = g_pTextureManager->GetGlobalTexture("HeightSpectrumRT");
		_GM->RHIBindTexture("OceanSurfaceHeightMap", OceanSurfaceHeightMap, params);
		params.Mag_Filter = ETextureFilters::TF_Linear;
		params.Min_Filter = ETextureFilters::TF_Linear;
		params.Wrap_S = ETextureWrapModes::TW_Clamp;
		params.Wrap_T = ETextureWrapModes::TW_Clamp;
		params.Wrap_R = ETextureWrapModes::TW_Clamp;
		RHITextureRef TransmittanceLUT = g_pTextureManager->GetGlobalTexture("TransmittanceLUT");
		_GM->RHIBindTexture("TransmittanceLUT", TransmittanceLUT, params);
		RHITextureRef SingleScatteringRayleighLUT = g_pTextureManager->GetGlobalTexture("SingleScatteringRayleighLUT");
		_GM->RHIBindTexture("SingleScatteringRayleighLUT", SingleScatteringRayleighLUT, params);
		RHITextureRef SingleScatteringMieLUT = g_pTextureManager->GetGlobalTexture("SingleScatteringMieLUT");
		_GM->RHIBindTexture("SingleScatteringMieLUT", SingleScatteringMieLUT, params);
		RHITextureRef OceanScatteringLUT = g_pTextureManager->GetGlobalTexture("OceanScatteringLUT");
		_GM->RHIBindTexture("OceanScatteringLUT", OceanScatteringLUT, params);
		RHITextureRef OceanScatteringDensityLUT = g_pTextureManager->GetGlobalTexture("OceanScatteringDensityLUT");
		_GM->RHIBindTexture("OceanScatteringDensityLUT", OceanScatteringDensityLUT, params);

		_GM->RHIDrawVertexArray(m_ScreenVAO);
		state.EnableDepthTest = true;
		_GM->RHISetDepthStencilState(state);
	}
}