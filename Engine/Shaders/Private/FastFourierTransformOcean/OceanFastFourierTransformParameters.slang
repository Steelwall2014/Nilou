module OceanFastFourierTransformParameters;

public struct FOceanFastFourierTransformParameters
{
    public float2 WindDirection;
    public uint N;
    public float WindSpeed;
    public float Amplitude;
    public float DisplacementTextureSize;
    public float Time;
}
public ParameterBlock<FOceanFastFourierTransformParameters> OceanFFTParameters;
