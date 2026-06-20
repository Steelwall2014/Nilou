module atmosphere_scattering_density_pre;

import unit_definitions;
import atmosphere_functions;
import atmosphere_definitions;
import atmosphere_constants;
import RenderCore;

struct FAtmosphereScatteringDensityParameters
{
    Texture2D<float4> TransmittanceLUT;
    Texture2D<float4> DeltaIrradianceLUT;
    Texture3D<float4> DeltaScatteringLUT;
    SamplerState linearSampler;
    RWTexture3D<float4> ScatteringDensityLUT;
    uint scattering_order;
};
ParameterBlock<FAtmosphereScatteringDensityParameters> Params;
ParameterBlock<AtmosphereParameters> ATMOSPHERE;

RadianceDensitySpectrum ComputeScatteringDensity(
    AtmosphereParameters atmosphere,
    TransmittanceTexture transmittance_texture,
    ScatteringTexture multiple_scattering_texture,
    IrradianceTexture irradiance_texture,
    SamplerState linearSampler,
    Length r, Number mu, Number mu_s, Number nu,
    uint scattering_order) {

  // Compute unit direction vectors for the zenith, the view direction omega and
  // and the sun direction omega_s, such that the cosine of the view-zenith
  // angle is mu, the cosine of the sun-zenith angle is mu_s, and the cosine of
  // the view-sun angle is nu. The goal is to simplify computations below.
  float3 zenith_direction = float3(0.0, 0.0, 1.0);
  float3 omega = float3(sqrt(1.0 - mu * mu), 0.0, mu);
  Number sun_dir_x = omega.x == 0.0 ? 0.0 : (nu - mu * mu_s) / omega.x;
  Number sun_dir_y = sqrt(max(1.0 - sun_dir_x * sun_dir_x - mu_s * mu_s, 0.0));
  float3 omega_s = float3(sun_dir_x, sun_dir_y, mu_s);

  static const int SAMPLE_COUNT = 16;
  static const Angle dphi = pi / Number(SAMPLE_COUNT);
  static const Angle dtheta = pi / Number(SAMPLE_COUNT);
  RadianceDensitySpectrum rayleigh_mie =
      RadianceDensitySpectrum(0.0 * watt_per_cubic_meter_per_sr_per_nm);

  // Nested loops for the integral over all the incident directions omega_i.
  for (int l = 0; l < SAMPLE_COUNT; ++l) {
    Angle theta = (Number(l) + 0.5) * dtheta;
    Number cos_theta = cos(theta);
    Number sin_theta = sin(theta);
    bool ray_r_theta_intersects_ground =
        RayIntersectsGround(atmosphere, r, cos_theta);

    // The distance and transmittance to the ground only depend on theta, so we
    // can compute them in the outer loop for efficiency.
    Length distance_to_ground = 0.0 * m;
    DimensionlessSpectrum transmittance_to_ground = DimensionlessSpectrum(0.0);
    DimensionlessSpectrum ground_albedo = DimensionlessSpectrum(0.0);
    if (ray_r_theta_intersects_ground) {
      distance_to_ground =
          DistanceToBottomAtmosphereBoundary(atmosphere, r, cos_theta);
      transmittance_to_ground =
          GetTransmittance(atmosphere, transmittance_texture, linearSampler, r, cos_theta,
              distance_to_ground, true /* ray_intersects_ground */);
      ground_albedo = atmosphere.ground_albedo;
    }

    for (int m = 0; m < 2 * SAMPLE_COUNT; ++m) {
      Angle phi = (Number(m) + 0.5) * dphi;
      float3 omega_i =
          float3(cos(phi) * sin_theta, sin(phi) * sin_theta, cos_theta);
      SolidAngle domega_i = (dtheta / rad) * (dphi / rad) * sin(theta) * sr;

      // The radiance L_i arriving from direction omega_i after n-1 bounces is
      // the sum of a term given by the precomputed scattering texture for the
      // (n-1)-th order:
      Number nu1 = dot(omega_s, omega_i);
      RadianceSpectrum incident_radiance;
      if (scattering_order == 2) {
        // The (n-1)-th order is single scattering, stored in the combined
        // convention (rgb = Rayleigh, alpha = Mie.r) without phase. Reconstruct
        // the actual radiance by applying the phase functions analytically.
        float4 single = GetScattering4(atmosphere, multiple_scattering_texture,
            linearSampler, r, omega_i.z, mu_s, nu1, ray_r_theta_intersects_ground);
        IrradianceSpectrum rayleigh = single.rgb;
        IrradianceSpectrum mie =
            GetExtrapolatedSingleMieScattering(atmosphere, single);
        incident_radiance = rayleigh * RayleighPhaseFunction(nu1) +
            mie * MiePhaseFunction(atmosphere.mie_phase_function_g, nu1);
      } else {
        // Higher orders read the per-order delta texture, which already holds
        // raw scattered radiance (phase already incorporated when computed).
        incident_radiance = GetScattering(atmosphere,
            multiple_scattering_texture, linearSampler, r, omega_i.z, mu_s, nu1,
            ray_r_theta_intersects_ground);
      }

      // and of the contribution from the light paths with n-1 bounces and whose
      // last bounce is on the ground. This contribution is the product of the
      // transmittance to the ground, the ground albedo, the ground BRDF, and
      // the irradiance received on the ground after n-2 bounces.
      float3 ground_normal =
          normalize(zenith_direction * r + omega_i * distance_to_ground);
      IrradianceSpectrum ground_irradiance = GetIrradiance(
          atmosphere, irradiance_texture, linearSampler, atmosphere.bottom_radius,
          dot(ground_normal, omega_s));
      incident_radiance += transmittance_to_ground *
          ground_albedo * (1.0 / (PI * sr)) * ground_irradiance;

      // The radiance finally scattered from direction omega_i towards direction
      // -omega is the product of the incident radiance, the scattering
      // coefficient, and the phase function for directions omega and omega_i
      // (all this summed over all particle types, i.e. Rayleigh and Mie).
      Number nu2 = dot(omega, omega_i);
      Number rayleigh_density = GetProfileDensity(
          atmosphere.rayleigh_density, r - atmosphere.bottom_radius);
      Number mie_density = GetProfileDensity(
          atmosphere.mie_density, r - atmosphere.bottom_radius);
      rayleigh_mie += incident_radiance * (
          atmosphere.rayleigh_scattering * rayleigh_density *
              RayleighPhaseFunction(nu2) +
          atmosphere.mie_scattering * mie_density *
              MiePhaseFunction(atmosphere.mie_phase_function_g, nu2)) *
          domega_i;
    }
  }
  return rayleigh_mie;
}

RadianceDensitySpectrum ComputeScatteringDensityTexture(
    AtmosphereParameters atmosphere,
    TransmittanceTexture transmittance_texture,
    ScatteringTexture multiple_scattering_texture,
    IrradianceTexture irradiance_texture,
    SamplerState linearSampler,
    float3 frag_coord, uint scattering_order) {
  Length r;
  Number mu;
  Number mu_s;
  Number nu;
  bool ray_r_mu_intersects_ground;
  GetRMuMuSNuFromScatteringTextureFragCoord(atmosphere, frag_coord,
      r, mu, mu_s, nu, ray_r_mu_intersects_ground);
  return ComputeScatteringDensity(atmosphere, transmittance_texture,
      multiple_scattering_texture, irradiance_texture, linearSampler, r, mu, mu_s, nu,
      scattering_order);
}


[shader("compute")]
[numthreads(8, 8, 8)]
void main(uint3 threadID : SV_DispatchThreadID)
{
    float3 frag_coord = float3(threadID) + float3(0.5);
    RadianceDensitySpectrum density_spectrum = ComputeScatteringDensityTexture(
        ATMOSPHERE, 
        Params.TransmittanceLUT, 
        Params.DeltaScatteringLUT,
        Params.DeltaIrradianceLUT, 
        Params.linearSampler, 
        frag_coord,
        Params.scattering_order);
    Params.ScatteringDensityLUT[threadID] = float4(density_spectrum, 0);
}

