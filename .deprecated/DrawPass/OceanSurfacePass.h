#pragma once
#include "Common/DrawPass/HugeSurfacePass.h"

namespace und {
	class OceanSurfacePass : implements HugeSurfacePass
	{
	private:

		RHITexture2DRef GaussianRandomRT;          // 高斯随机数
		RHITexture2DRef HeightSpectrumRT;          // 高度频谱
		RHITexture2DRef DisplaceXSpectrumRT;       // X偏移频谱
		RHITexture2DRef DisplaceYSpectrumRT;       // Y偏移频谱
		RHITexture2DRef DisplaceRT;                // 偏移频谱
		RHITexture2DRef OutputRT;                  // 临时储存输出纹理
		RHITexture2DRef NormalRT;                  // 法线
		RHITexture2DRef FoamRT;					   // 白沫
		RHITexture2DRef SlopeVariance;			   // 坡度方差

		RHITexture2DRef PerlinNoise;
#ifdef _DEBUG
		bool PauseOceanSurface = false;
		bool InputColors = false;
#endif // _DEBUG

		void Occean_ComputeGaussionSpectrums(unsigned int N, int group_x, int group_y);
		void Ocean_ComputeDisplacementSpectrums(unsigned int N, glm::vec2 WindDirection, float WindSpeed, float TimeToNow, float A, float Lxy, int group_x, int group_y);
		void Ocean_ComputeFFT(und::RHITexture2DRef &input, unsigned int N, unsigned int Ns, int function, int group_x, int group_y);
		void Ocean_ComputeDisplacement(unsigned int N, float Scale, int group_x, int group_y);
		void Ocean_ComputeNormalFoam(unsigned int N, float grid_size, int group_x, int group_y);
		void Ocean_UpdateOceanSurfaceTextures(float time);

	public:
		virtual int Initialize(FrameVariables &frame) override;
		virtual void Draw(FrameVariables &frame) override;

#ifdef _DEBUG
		void SwitchPauseOceanSurface();
		void SwitchInputColors();
#endif // _DEBUG
	};
}