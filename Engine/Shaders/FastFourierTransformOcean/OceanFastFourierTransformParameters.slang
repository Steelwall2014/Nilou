struct FOceanFastFourierTransformParameters
{
    float2 WindDirection;
    uint N;
    float WindSpeed;
    float Amplitude;
    float DisplacementTextureSize;
    float Time;
}
ParameterBlock<FOceanFastFourierTransformParameters> OceanFFTParameters;

