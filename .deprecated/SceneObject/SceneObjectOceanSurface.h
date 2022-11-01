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
		int			FFTPow;				// �������ƺ���FFT�Ĵ�С������FFTPow=10ʱFFT��СΪ1024*1024
		float		A;					// phillipsƵ���е�A
		float		FFTPixelMeterSize;			
		SceneObjectOceanSurface(
			glm::vec2 wind_direction, float wind_speed, 
			int fft_pow, unsigned int LODNum=7, unsigned int TopLODNodeSideNum=5, float TopLODNodeMeterSize=8192.0f, float A = 0.5f);
	};
}