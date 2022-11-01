#pragma once
#include "SceneObjectHugeSurface.h"
#include "Common/QuadTree/QuadTree.h"


namespace und {
	class SceneObjectMesh;
	class SceneObjectOceanSurface : public SceneObjectHugeSurface
	{
	public:
		glm::vec2	WindDirection;
		float		WindSpeed;
		int			FFTPow;				// 用来控制海面FFT的大小，比如FFTPow=10时FFT大小为1024*1024
		float		A;					// phillips频谱中的A
		float		FFTPixelMeterSize;			
		SceneObjectOceanSurface(
			glm::vec2 wind_direction, float wind_speed, 
			int fft_pow, unsigned int LODNum=7, unsigned int TopLODNodeSideNum=5, float TopLODNodeMeterSize=8192.0f, float A = 0.5f);
	};
}