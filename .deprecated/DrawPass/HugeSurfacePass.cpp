#include "DynamicRHI.h"

#include "Common/SceneObject/SceneObjectHugeSurface.h"
#include "Common/ShaderManager.h"
#include "OpenGL/OpenGLUtils.h"
#include "HugeSurfacePass.h"
#include "QuadTreeSubPass.h"

#ifdef _DEBUG
#include "Common/InputManager.h"
#endif // _DEBUG

void und::HugeSurfacePass::HugeSurfaceInit(std::shared_ptr<SceneObjectHugeSurface> surface)
{
	surface_dbc = CreateDBCFromMesh(*surface->PatchMesh);
	m_Surface = surface;

	m_QuadUpdatePass = new QuadTreeUpdateSubPass;
	m_QuadUpdatePass->Initialize(*surface->QTree);

	m_DrawIndirectArgs = _GM->RHICreateDrawElementsIndirectBuffer(surface_dbc.vao);

#ifdef _DEBUG
	InputActionMapping show_ocean("ShowOceanSurface");
	show_ocean.AddGroup(InputKey::KEY_O);
	g_pInputManager->BindAction(show_ocean, InputEvent::IE_Pressed, this, &HugeSurfacePass::SwitchOceanSurface);

	InputActionMapping show_grid_ocean("ShowGridOceanSurface");
	show_grid_ocean.AddGroup(InputKey::KEY_G);
	g_pInputManager->BindAction(show_grid_ocean, InputEvent::IE_Pressed, this, &HugeSurfacePass::SwitchOceanGridSurface);

	InputActionMapping show_boundingbox("ShowBoundingBox");
	show_boundingbox.AddGroup(InputKey::KEY_B);
	g_pInputManager->BindAction(show_boundingbox, InputEvent::IE_Pressed, this, &HugeSurfacePass::SwitchBoundingBox);
#endif // _DEBUG

#ifdef DRAW_BOUNDINGBOX
	float vertices[] = {
		-0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f, -0.5f,
		 0.5f,  0.5f, -0.5f,
		 0.5f,  0.5f, -0.5f,
		-0.5f,  0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,

		-0.5f, -0.5f,  0.5f,
		 0.5f, -0.5f,  0.5f,
		 0.5f,  0.5f,  0.5f,
		 0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,
		-0.5f, -0.5f,  0.5f,

		-0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,

		 0.5f,  0.5f,  0.5f,
		 0.5f,  0.5f, -0.5f,
		 0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f,  0.5f,
		 0.5f,  0.5f,  0.5f,

		-0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f,  0.5f,
		 0.5f, -0.5f,  0.5f,
		-0.5f, -0.5f,  0.5f,
		-0.5f, -0.5f, -0.5f,

		-0.5f,  0.5f, -0.5f,
		 0.5f,  0.5f, -0.5f,
		 0.5f,  0.5f,  0.5f,
		 0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f, -0.5f,
	};
	m_BoxVAO = _GM->RHICreateVertexArrayObject(EPrimitiveMode::PM_Triangles);
	auto VBO = _GM->RHICreateVertexAttribBuffer(0, sizeof(vertices), vertices, EVertexElementTypeFlags::VET_Float | EVertexElementTypeFlags::VET_3, EBufferUsageFlags::VertexBuffer | EBufferUsageFlags::Static);
	m_BoxVAO->AddVertexAttribBuffer(VBO.first, VBO.second);
	_GM->RHIInitializeVertexArrayObject(m_BoxVAO);
	std::vector<WorldLODParam> LODParams = surface->QTree->LODParams;
	m_LODParamsBuffer = _GM->RHICreateShaderStorageBuffer(LODParams.size() * sizeof(und::WorldLODParam), LODParams.data());

#endif // DRAW_BOUNDINGBOX

}

void und::HugeSurfacePass::HugeSurfaceUpdatePatchList(FrameVariables &frame, und::RHITexture2DRef HeightMap, und::RHITexture2DRef DisplacementMap, float HeightMapMeterSize, ETextureWrapModes HeightMapWrapMode)
{
	m_QuadUpdatePass->DrawOrCompute(frame, *m_Surface->QTree, HeightMap, DisplacementMap, HeightMapMeterSize, HeightMapWrapMode);

#ifdef DIRECT_DISPATCH
	m_PatchNumToDraw = m_QuadUpdatePass->AtomicPatchCounter;
#else
	_GM->RHICopyBufferSubData(m_QuadUpdatePass->AtomicPatchCounterBuffer, m_DrawIndirectArgs, 0, 4, 4);
#endif // DIRECT_DISPATCH

	m_CulledPatchListBuffer = m_QuadUpdatePass->CulledPatchListBuffer;
}

int und::HugeSurfacePass::Initialize(FrameVariables &frame)
{
	return 0;
}

void und::HugeSurfacePass::Draw(FrameVariables &frame)
{
#ifdef _DEBUG
	if (!ShowOceanSurface)
		return;
#endif // _DEBUG


	RHIRasterizerState state;


    _GM->GLDEBUG();
#ifdef _DEBUG
	if (ShowGridOceanSurface)
	{
		state.PolyMode_Face = ERasterizerPolyMode_Face::PMF_Front_and_Back;
		state.PolyMode_Mode = ERasterizerPolyMode_Mode::PMM_LINE;
		_GM->RHISetRasterizerState(state);
	}
#endif // _DEBUG

	SetPerFrameShaderParameters(frame.frameContext);

	_GM->RHISetShaderParameter("HeightMapMeterSize", m_Surface->HeightMapMeterSize);
	_GM->RHISetShaderParameter("PatchOriginalGridMeterSize", m_Surface->PatchOriginalGridMeterSize);
	_GM->RHISetShaderParameter("PatchGridSideNum", m_Surface->PatchGridSideNum);

	_GM->RHIBindVertexArrayObject(surface_dbc.vao);
	_GM->RHIBindComputeBuffer(4, m_CulledPatchListBuffer);

#ifdef DIRECT_DISPATCH
	_GM->RHIDrawVertexArrayInstanced(surface_dbc.vao, m_PatchNumToDraw, frame.frameContext.renderTarget);
#else
	_GM->RHIDrawVertexArrayIndirect(surface_dbc.vao, m_DrawIndirectArgs, frame.frameContext.renderTarget);
#endif

	state.PolyMode_Face = ERasterizerPolyMode_Face::PMF_Front_and_Back;
	state.PolyMode_Mode = ERasterizerPolyMode_Mode::PMM_FILL;
	_GM->RHISetRasterizerState(state);


#ifdef DRAW_BOUNDINGBOX
	if (!ShowBoundingBox)
		return;
	_GM->RHIUseShaderProgram(g_pShaderManager->GetShaderByName("Debug BoundingBox"));
	state.EnableCull = false;
	state.PolyMode_Mode = ERasterizerPolyMode_Mode::PMM_LINE;
	_GM->RHISetRasterizerState(state);
	char uniformName[32];
	for (int LOD = 0; LOD < m_Surface->QTree->LODNum; LOD++)
	{
		sprintf(uniformName, "MinMaxMap[%d]", LOD);
                RHITextureParams params;
                params.Mag_Filter = ETextureFilters::TF_Nearest;
		params.Min_Filter = ETextureFilters::TF_Nearest;
		_GM->RHIBindTexture(uniformName, m_QuadUpdatePass->MinMaxMap[LOD], params);
	}

	_GM->RHIBindComputeBuffer(1, m_CulledPatchListBuffer);
	_GM->RHIBindComputeBuffer(2, m_LODParamsBuffer);

	und::Frustum &frustum = frame.frameContext.cameraFrustum;
	for (int i = 0; i < 6; i++)
	{
		sprintf(uniformName, "FrustumPlanes[%d]", i);
		_GM->RHISetShaderParameter(uniformName, frustum.Planes[i]);
	}

	_GM->RHISetShaderParameter("VP", frame.frameContext.projectionMatrix * frame.frameContext.viewMatrix);

#ifdef DIRECT_DISPATCH
	unsigned int *ptr = (unsigned int *)_GM->RHIMapComputeBuffer(m_QuadUpdatePass->AtomicPatchCounterBuffer, EDataAccessFlag::DA_ReadOnly);
	unsigned int patch_num = *ptr;
	_GM->RHIUnmapComputeBuffer(m_QuadUpdatePass->AtomicPatchCounterBuffer);
	_GM->RHIDrawVertexArrayInstanced(m_BoxVAO, patch_num, frame.frameContext.renderTarget);
#else
	_GM->RHIDrawVertexArrayIndirect(m_BoxVAO, m_DrawIndirectArgs, frame.frameContext.renderTarget);
#endif

	state.EnableCull = true;
	state.PolyMode_Mode = ERasterizerPolyMode_Mode::PMM_FILL;
	_GM->RHISetRasterizerState(state);
#endif // DRAW_BOUNDINGBOX
}

#ifdef _DEBUG
void und::HugeSurfacePass::SwitchOceanSurface()
{
	ShowOceanSurface = !ShowOceanSurface;
}
void und::HugeSurfacePass::SwitchOceanGridSurface()
{
	ShowGridOceanSurface = !ShowGridOceanSurface;
}

void und::HugeSurfacePass::SwitchBoundingBox()
{
	ShowBoundingBox = !ShowBoundingBox;
}
#endif
