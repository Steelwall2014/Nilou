module atmosphere_indirect_irradiance_pre;

import unit_definitions;
import atmosphere_functions;
import atmosphere_definitions;
import atmosphere_constants;
import RenderCore;

struct FAtmosphereIndirectIrradianceParameters
{
    int scattering_order;
    Texture3D<float4> SingleScatteringRayleighLUT;
    Texture3D<float4> SingleScatteringMieLUT;
    SamplerState linearSampler;
    RWTexture2D<float4> IrradianceLUT;
};
ParameterBlock<FAtmosphereIndirectIrradianceParameters> Params;
ParameterBlock<AtmosphereParameters> ATMOSPHERE;

IrradianceSpectrum ComputeIndirectIrradiance(
    AtmosphereParameters atmosphere,
    ReducedScatteringTexture single_rayleigh_scattering_texture,
    ReducedScatteringTexture single_mie_scattering_texture,
    ScatteringTexture multiple_scattering_texture,
    SamplerState linearSampler,
    Length r, Number mu_s, int scattering_order) {

  static const int SAMPLE_COUNT = 32;
  static const Angle dphi = pi / Number(SAMPLE_COUNT);
  static const Angle dtheta = pi / Number(SAMPLE_COUNT);

  IrradianceSpectrum result =
      IrradianceSpectrum(0.0 * watt_per_square_meter_per_nm);
  float3 omega_s = float3(sqrt(1.0 - mu_s * mu_s), 0.0, mu_s);
  for (int j = 0; j < SAMPLE_COUNT / 2; ++j) {
    Angle theta = (Number(j) + 0.5) * dtheta;
    for (int i = 0; i < 2 * SAMPLE_COUNT; ++i) {
      Angle phi = (Number(i) + 0.5) * dphi;
      float3 omega =
          float3(cos(phi) * sin(theta), sin(phi) * sin(theta), cos(theta));
      SolidAngle domega = (dtheta / rad) * (dphi / rad) * sin(theta) * sr;

      Number nu = dot(omega, omega_s);
      result += GetScattering(atmosphere, single_rayleigh_scattering_texture,
          single_mie_scattering_texture, multiple_scattering_texture,
          linearSampler, r, omega.z, mu_s, nu, false /* ray_r_theta_intersects_ground */,
          scattering_order) *
              omega.z * domega;
    }
  }
  return result;
}

IrradianceSpectrum ComputeIndirectIrradianceTexture(
    AtmosphereParameters atmosphere,
    ReducedScatteringTexture single_rayleigh_scattering_texture,
    ReducedScatteringTexture single_mie_scattering_texture,
    ScatteringTexture multiple_scattering_texture,
    SamplerState linearSampler,
    float2 frag_coord, int scattering_order) {
  Length r;
  Number mu_s;
  GetRMuSFromIrradianceTextureUv(
      atmosphere, frag_coord / IRRADIANCE_TEXTURE_SIZE, r, mu_s);
  return ComputeIndirectIrradiance(atmosphere,
      single_rayleigh_scattering_texture, single_mie_scattering_texture,
      multiple_scattering_texture, linearSampler, r, mu_s, scattering_order);
}

[shader("compute")]
[numthreads(8, 8, 1)]
void main(uint3 threadID : SV_DispatchThreadID)
{
    float2 frag_coord = float2(threadID.xy) + float2(0.5);
    IrradianceSpectrum irradiance = ComputeIndirectIrradianceTexture(
        ATMOSPHERE, 
        Params.SingleScatteringRayleighLUT, 
        Params.SingleScatteringMieLUT, 
        Params.SingleScatteringRayleighLUT, 
        Params.linearSampler, 
        frag_coord, 
        Params.scattering_order);
    Params.IrradianceLUT[threadID.xy] = float4(irradiance, 0);
}

