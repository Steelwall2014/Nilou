module atmosphere_transmittance_pre;

import unit_definitions;
import atmosphere_functions;
import atmosphere_definitions;
import atmosphere_constants;
import RenderCore;

struct FAtmosphereTransmittanceParameters
{
    RWTexture2D<float4> TransmittanceLUT;
};
ParameterBlock<FAtmosphereTransmittanceParameters> Params;
ParameterBlock<AtmosphereParameters> ATMOSPHERE;

[shader("compute")]
[numthreads(8, 8, 1)]
void main(uint3 threadID : SV_DispatchThreadID)
{
    float2 frag_coord = float2(threadID.xy) + float2(0.5);
    DimensionlessSpectrum transmittance = ComputeTransmittanceToTopAtmosphereBoundaryTexture(ATMOSPHERE, frag_coord);
    Params.TransmittanceLUT[threadID.xy] = float4(transmittance, 0.0);
}

Length ComputeOpticalLengthToTopAtmosphereBoundary(
    AtmosphereParameters atmosphere, DensityProfile profile,
    Length r, Number mu) {
  // Number of intervals for the numerical integration.
  static const int SAMPLE_COUNT = 500;
  // The integration step, i.e. the length of each integration interval.
  Length dx =
      DistanceToTopAtmosphereBoundary(atmosphere, r, mu) / Number(SAMPLE_COUNT);
  // Integration loop.
  Length result = 0.0 * m;
  for (int i = 0; i <= SAMPLE_COUNT; ++i) {
    Length d_i = Number(i) * dx;
    // Distance between the current sample point and the planet center.
    Length r_i = sqrt(d_i * d_i + 2.0 * r * mu * d_i + r * r);
    // Number density at the current sample point (divided by the number density
    // at the bottom of the atmosphere, yielding a dimensionless number).
    Number y_i = GetProfileDensity(profile, r_i - atmosphere.bottom_radius);
    // Sample weight (from the trapezoidal rule).
    Number weight_i = i == 0 || i == SAMPLE_COUNT ? 0.5 : 1.0;
    result += y_i * weight_i * dx;
  }
  return result;
}

DimensionlessSpectrum ComputeTransmittanceToTopAtmosphereBoundary(
    AtmosphereParameters atmosphere, Length r, Number mu) {
  return exp(-(
      atmosphere.rayleigh_scattering *
          ComputeOpticalLengthToTopAtmosphereBoundary(
              atmosphere, atmosphere.rayleigh_density, r, mu) +
      atmosphere.mie_extinction *
          ComputeOpticalLengthToTopAtmosphereBoundary(
              atmosphere, atmosphere.mie_density, r, mu) +
      atmosphere.absorption_extinction *
          ComputeOpticalLengthToTopAtmosphereBoundary(
              atmosphere, atmosphere.absorption_density, r, mu)));
}

DimensionlessSpectrum ComputeTransmittanceToTopAtmosphereBoundaryTexture(
    AtmosphereParameters atmosphere, float2 frag_coord) {
  static const float2 TRANSMITTANCE_TEXTURE_SIZE =
      float2(TRANSMITTANCE_TEXTURE_WIDTH, TRANSMITTANCE_TEXTURE_HEIGHT);
  Length r;
  Number mu;
  GetRMuFromTransmittanceTextureUv(
      atmosphere, frag_coord / TRANSMITTANCE_TEXTURE_SIZE, r, mu);
  return ComputeTransmittanceToTopAtmosphereBoundary(atmosphere, r, mu);
}

