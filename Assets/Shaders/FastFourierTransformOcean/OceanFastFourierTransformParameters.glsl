#ifndef OCEAN_FFT_PARAMETERS_H
#define OCEAN_FFT_PARAMETERS_H

layout (std140) uniform FOceanFastFourierTransformParameters {
	vec2 WindDirection;
	uint N;
	float WindSpeed;
	float Amplitude;
	float DisplacementTextureSize;
	float Time;
};

#endif