module atmosphere_multiscattering_pre;

import unit_definitions;
import atmosphere_functions;
import atmosphere_definitions;
import atmosphere_constants;
import RenderCore;

struct FAtmosphereMultiScatteringParameters
{
    Texture2D<float4> TransmittanceLUT;
    Texture3D<float4> ScatteringDensityLUT;
    SamplerState linearSampler;
    RWTexture3D<float4> ScatteringLUT;
    RWTexture3D<float4> DeltaScatteringLUT;
};
ParameterBlock<FAtmosphereMultiScatteringParameters> Params;
ParameterBlock<AtmosphereParameters> ATMOSPHERE;

RadianceSpectrum ComputeMultipleScattering(
    AtmosphereParameters atmosphere,
    TransmittanceTexture transmittance_texture,
    ScatteringDensityTexture scattering_density_texture,
    SamplerState linearSampler,
    Length r, Number mu, Number mu_s, Number nu,
    bool ray_r_mu_intersects_ground) {

  // Number of intervals for the numerical integration.
  static const int SAMPLE_COUNT = 50;
  // The integration step, i.e. the length of each integration interval.
  Length dx =
      DistanceToNearestAtmosphereBoundary(
          atmosphere, r, mu, ray_r_mu_intersects_ground) /
              Number(SAMPLE_COUNT);
  // Integration loop.
  RadianceSpectrum rayleigh_mie_sum =
      RadianceSpectrum(0.0 * watt_per_square_meter_per_sr_per_nm);
  for (int i = 0; i <= SAMPLE_COUNT; ++i) {
    Length d_i = Number(i) * dx;

    // The r, mu and mu_s parameters at the current integration point (see the
    // single scattering section for a detailed explanation).
    Length r_i =
        ClampRadius(atmosphere, sqrt(d_i * d_i + 2.0 * r * mu * d_i + r * r));
    Number mu_i = ClampCosine((r * mu + d_i) / r_i);
    Number mu_s_i = ClampCosine((r * mu_s + d_i * nu) / r_i);

    // The Rayleigh and Mie multiple scattering at the current sample point.
    RadianceSpectrum rayleigh_mie_i =
        RadianceSpectrum(GetScattering(
            atmosphere, scattering_density_texture, linearSampler, r_i, mu_i, mu_s_i, nu,
            ray_r_mu_intersects_ground)) *
        GetTransmittance(
            atmosphere, transmittance_texture, linearSampler, r, mu, d_i,
            ray_r_mu_intersects_ground) *
        dx;
    // Sample weight (from the trapezoidal rule).
    Number weight_i = (i == 0 || i == SAMPLE_COUNT) ? 0.5 : 1.0;
    rayleigh_mie_sum += rayleigh_mie_i * weight_i;
  }
  return rayleigh_mie_sum;
}

RadianceSpectrum ComputeMultipleScatteringTexture(
    AtmosphereParameters atmosphere,
    TransmittanceTexture transmittance_texture,
    ScatteringDensityTexture scattering_density_texture,
    SamplerState linearSampler,
    float3 frag_coord, out Number nu) {
  Length r;
  Number mu;
  Number mu_s;
  bool ray_r_mu_intersects_ground;
  GetRMuMuSNuFromScatteringTextureFragCoord(atmosphere, frag_coord,
      r, mu, mu_s, nu, ray_r_mu_intersects_ground);
  return ComputeMultipleScattering(atmosphere, transmittance_texture,
      scattering_density_texture, linearSampler, r, mu, mu_s, nu,
      ray_r_mu_intersects_ground);
}

[shader("compute")]
[numthreads(8, 8, 8)]
void main(uint3 threadID : SV_DispatchThreadID)
{
    Number nu;
    float3 frag_coord = float3(threadID) + float3(0.5);
    RadianceSpectrum spectrum = ComputeMultipleScatteringTexture(
        ATMOSPHERE, 
        Params.TransmittanceLUT, 
        Params.ScatteringDensityLUT, 
        Params.linearSampler, 
        frag_coord, 
        nu);
    Params.ScatteringLUT[threadID] += float4(spectrum, 0);
    Params.DeltaScatteringLUT[threadID] = float4(spectrum, 0);
}

