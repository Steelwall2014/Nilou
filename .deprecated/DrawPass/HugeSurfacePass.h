#pragma once
#include "Interface/IDrawPass.h"
#include "Common/GfxStructures.h"
#include "RHIResources.h"

#ifdef _DEBUG
#define DRAW_BOUNDINGBOX
#endif

namespace und {
	class QuadTreeUpdateSubPass;
	class SceneObjectHugeSurface;
	class HugeSurfacePass : implements IDrawPass
	{
	private:
#ifdef DRAW_BOUNDINGBOX
		RHIVertexArrayObjectRef m_BoxVAO;
		RHIBufferRef m_LODParamsBuffer;
#endif // DRAW_BOUNDINGBOX
		RHIBufferRef m_DrawIndirectArgs;
	protected:
		QuadTreeUpdateSubPass *m_QuadUpdatePass;
		DrawBatchContext surface_dbc;
		RHIBufferRef m_CulledPatchListBuffer;
		std::shared_ptr<SceneObjectHugeSurface> m_Surface;
		unsigned int m_PatchNumToDraw;
		void HugeSurfaceInit(std::shared_ptr<SceneObjectHugeSurface> surface);

		// HeightMap的r通道必须是高度，DisplacementMap的xy通道必须是水平偏移，如果DisplacementMap是nullptr，那就代表没有水平偏移
		void HugeSurfaceUpdatePatchList(FrameVariables &frame, RHITexture2DRef HeightMap, RHITexture2DRef DisplacementMap, float HeightMapMeterSize, ETextureWrapModes HeightMapWrapMode);

#ifdef _DEBUG
		bool ShowOceanSurface = true;
		bool ShowGridOceanSurface = false;
		bool ShowBoundingBox = false;
		
		void SwitchOceanSurface();
		void SwitchOceanGridSurface();
		void SwitchBoundingBox();
#endif // _DEBUG

	public:
		virtual int Initialize(FrameVariables &frame) override;
		virtual void Draw(FrameVariables &frame) override;

	};
}
