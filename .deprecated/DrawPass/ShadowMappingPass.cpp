#include <glm/glm.hpp>
#include "Common/ShaderManager.h"
#include "Common/WorldVectors.h"
#include "DynamicRHI.h"
#include "Interface/IApplication.h"

#include "ShadowMappingPass.h"

void und::ShadowMappingPass::Draw(FrameVariables &frame)
{
	// TODO: cascaded shadow map
	_GM->RHIUseShaderProgram(g_pShaderManager->GetShaderByName("Shadow map"));

	unsigned int shadowMapCount = 0;
	std::vector<Light *> lights_cast_shadow;
	for (auto &&light : frame.frameContext.lights)
	{
		if (light.lightCastShadow)
		{
			shadowMapCount++;
			lights_cast_shadow.push_back(&light);
		}
	}

	// 这里暂时是所有shadow map共用一个分辨率，如果有时间的话改掉
	const int kShadowMapWidth = frame.frameContext.lights[0].shadowMapResolution.x;
	const int kShadowMapHeight = frame.frameContext.lights[0].shadowMapResolution.y;

	frame.frameContext.shadowMapArray = _GM->RHICreateTexture2DArray("ShadowMap",
		EPixelFormat::PF_D32F, 1, kShadowMapWidth, kShadowMapHeight, shadowMapCount, nullptr);

	//return shadowMap;
	//frame.frameContext.shadowMapArray = GDynamicRHI->GenerateShadowMapArray(
	//	,
	//	,
	//	);

	for (int i = 0; i < lights_cast_shadow.size(); i++)
	{
		auto &light = lights_cast_shadow[i];
		int kShadowMapWidth = light->shadowMapResolution.x;
		int kShadowMapHeight = light->shadowMapResolution.y;
		//glCullFace(GL_FRONT);
		RHIDepthStencilState state;
		state.EnableDepthMask = true;
		_GM->RHISetDepthStencilState(state);
		auto ShadowMapFramebuffer = _GM->RHICreateFramebuffer(EFramebufferAttachment::FA_Depth_Attachment, frame.frameContext.shadowMapArray, i);
		_GM->RHIBindFramebuffer(ShadowMapFramebuffer);
		_GM->RHIClearBuffer(DEPTH_BUFFER_BIT);
		_GM->RHISetViewport(0, 0, kShadowMapWidth, kShadowMapHeight);

		// now set per frame constant
		glm::mat4 view = glm::lookAt(light->lightPosition, light->lightPosition + glm::vec3(light->lightDirection), und::WORLD_UP);

		// TODO 根据场景的大小自动推断fov
		float fieldOfView = glm::radians(90.f);
		float screenAspect = (float)kShadowMapWidth / (float)kShadowMapHeight;

		// Build the perspective projection matrix.
		glm::mat4 projection;
		if (light->lightType == LightType::Directional)
		{
			// 这里除以75是因为如果用方向光的话，下面的参数应该是整个场景正交投影到光源上，在那个平面上的场景的长宽
			// 这里实际应该用场景的包围盒算长宽
			float width = (float)kShadowMapWidth / 75.f;
			float height = (float)kShadowMapHeight / 75.f;
			projection = glm::ortho(-width, width, -height, height, light->nearClipDistance, light->farClipDistance);
		}
		else
		{
			projection = glm::perspective(fieldOfView, screenAspect, light->nearClipDistance, light->farClipDistance);
		}


		light->lightVP = projection * view;
		_GM->RHISetShaderParameter("lightVP", light->lightVP);
		//GDynamicRHI->BeginShadowMap(*lights_cast_shadow[i], frame.frameContext.shadowMapArray, i);
		for (auto &&dbc : frame.batchContexts)
		{
			_GM->RHISetShaderParameter("model", dbc.modelMatrix);
			_GM->RHIBindVertexArrayObject(dbc.vao);
			//RHIBindIndexArrayBuffer(context.vao->IndexBuffer);
			_GM->RHIDrawVertexArray(dbc.vao, ShadowMapFramebuffer);
			//glDrawElements(dbc.vao->IndexBuffer->Mode, dbc.vao->IndexBuffer->Count, dbc.vao->IndexBuffer->DataType, 0);
			//GDynamicRHI->DrawBatchDepthOnly(dbc);
		}
		_GM->RHIBindFramebuffer(nullptr);
		//glDeleteFramebuffers(1, &m_ShadowMapFramebuffer);

		const GfxConfiguration &conf = g_pApp->GetConfiguration();
		_GM->RHISetViewport(0, 0, conf.screenWidth, conf.screenHeight);
		//GDynamicRHI->EndShadowMap(frame.frameContext.shadowMapArray);
		lights_cast_shadow[i]->lightShadowMapLayerIndex = i;
	}
}
