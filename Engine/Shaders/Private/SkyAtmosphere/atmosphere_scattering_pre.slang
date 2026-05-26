module atmosphere_scattering_pre;

import unit_definitions;
import atmosphere_functions;
import atmosphere_definitions;
import atmosphere_constants;
import RenderCore;

struct FAtmosphereScatteringParameters
{
    Texture2D<float4> TransmittanceLUT;
    SamplerState linearSampler;
    RWTexture3D<float4> ScatteringLUT;
};
ParameterBlock<FAtmosphereScatteringParameters> Params;
ParameterBlock<AtmosphereParameters> ATMOSPHERE;

void ComputeSingleScatteringIntegrand(
    AtmosphereParameters atmosphere,
    TransmittanceTexture transmittance_texture,
    SamplerState linearSampler,
    Length r, Number mu, Number mu_s, Number nu, Length d,
    bool ray_r_mu_intersects_ground,
    out DimensionlessSpectrum rayleigh, out DimensionlessSpectrum mie) {
  Length r_d = ClampRadius(atmosphere, SafeSqrt(d * d + 2.0 * r * mu * d + r * r));
  Number mu_s_d = ClampCosine((r * mu_s + d * nu) / r_d);
  DimensionlessSpectrum transmittance =
      GetTransmittance(
          atmosphere, transmittance_texture, linearSampler, r, mu, d,
          ray_r_mu_intersects_ground) *
      GetTransmittanceToSun(
          atmosphere, transmittance_texture, linearSampler, r_d, mu_s_d);
  rayleigh = transmittance * GetProfileDensity(
      atmosphere.rayleigh_density, r_d - atmosphere.bottom_radius);
  mie = transmittance * GetProfileDensity(
      atmosphere.mie_density, r_d - atmosphere.bottom_radius);
}

void ComputeSingleScattering(
    AtmosphereParameters atmosphere,
    TransmittanceTexture transmittance_texture,
    SamplerState linearSampler,
    Length r, Number mu, Number mu_s, Number nu,
    bool ray_r_mu_intersects_ground,
    out IrradianceSpectrum rayleigh, out IrradianceSpectrum mie) {

  // Number of intervals for the numerical integration.
  static const int SAMPLE_COUNT = 50;
  // The integration step, i.e. the length of each integration interval.
  Length dx =
      DistanceToNearestAtmosphereBoundary(atmosphere, r, mu,
          ray_r_mu_intersects_ground) / Number(SAMPLE_COUNT);
  // Integration loop.
  DimensionlessSpectrum rayleigh_sum = DimensionlessSpectrum(0.0);
  DimensionlessSpectrum mie_sum = DimensionlessSpectrum(0.0);
  for (int i = 0; i <= SAMPLE_COUNT; ++i) {
    Length d_i = Number(i) * dx;
    // The Rayleigh and Mie single scattering at the current sample point.
    DimensionlessSpectrum rayleigh_i;
    DimensionlessSpectrum mie_i;
    ComputeSingleScatteringIntegrand(atmosphere, transmittance_texture, linearSampler,
        r, mu, mu_s, nu, d_i, ray_r_mu_intersects_ground, rayleigh_i, mie_i);
    // Sample weight (from the trapezoidal rule).
    Number weight_i = (i == 0 || i == SAMPLE_COUNT) ? 0.5 : 1.0;
    rayleigh_sum += rayleigh_i * weight_i;
    mie_sum += mie_i * weight_i;
  }
  rayleigh = rayleigh_sum * dx * atmosphere.solar_irradiance *
      atmosphere.rayleigh_scattering;
  mie = mie_sum * dx * atmosphere.solar_irradiance * atmosphere.mie_scattering;
}


[shader("compute")]
[numthreads(8, 8, 8)]
void main(uint3 threadID : SV_DispatchThreadID)
{
    IrradianceSpectrum rayleigh, mie;
    float3 frag_coord = float3(threadID) + float3(0.5);
    Length r;
    Number mu;
    Number mu_s;
    Number nu;
    bool ray_r_mu_intersects_ground;
    GetRMuMuSNuFromScatteringTextureFragCoord(ATMOSPHERE, frag_coord,
        r, mu, mu_s, nu, ray_r_mu_intersects_ground);
    ComputeSingleScattering(ATMOSPHERE, Params.TransmittanceLUT, Params.linearSampler,
        r, mu, mu_s, nu, ray_r_mu_intersects_ground, rayleigh, mie);
    Params.ScatteringLUT[threadID] = float4(
        rayleigh * RayleighPhaseFunction(nu) +
        mie * MiePhaseFunction(ATMOSPHERE.mie_phase_function_g, nu), 0);
}

