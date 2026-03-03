module atmosphere_direct_irradiance_pre;

import unit_definitions;
import atmosphere_functions;
import atmosphere_definitions;
import atmosphere_constants;
import RenderCore;

struct FAtmosphereDirectIrradianceParameters
{
    Texture2D<float4> TransmittanceLUT;
    SamplerState linearSampler;
    RWTexture2D<float4> IrradianceLUT;
};
ParameterBlock<FAtmosphereDirectIrradianceParameters> Params;
ParameterBlock<AtmosphereParameters> ATMOSPHERE;

IrradianceSpectrum ComputeDirectIrradiance(
    AtmosphereParameters atmosphere,
    TransmittanceTexture transmittance_texture,
    SamplerState linearSampler,
    Length r, Number mu_s) {

  Number alpha_s = atmosphere.sun_angular_radius / rad;
  // Approximate average of the cosine factor mu_s over the visible fraction of
  // the Sun disc.
  Number average_cosine_factor =
    mu_s < -alpha_s ? 0.0 : (mu_s > alpha_s ? mu_s :
        (mu_s + alpha_s) * (mu_s + alpha_s) / (4.0 * alpha_s));

  return atmosphere.solar_irradiance *
      GetTransmittanceToTopAtmosphereBoundary(
          atmosphere, transmittance_texture, linearSampler, r, mu_s) * average_cosine_factor;

}

IrradianceSpectrum ComputeDirectIrradianceTexture(
    AtmosphereParameters atmosphere,
    TransmittanceTexture transmittance_texture,
    SamplerState linearSampler,
    float2 frag_coord) {
  Length r;
  Number mu_s;
  GetRMuSFromIrradianceTextureUv(
      atmosphere, frag_coord / IRRADIANCE_TEXTURE_SIZE, r, mu_s);
  return ComputeDirectIrradiance(atmosphere, transmittance_texture, linearSampler, r, mu_s);
}


[shader("compute")]
[numthreads(8, 8, 1)]
void main(uint3 threadID : SV_DispatchThreadID)
{
    float2 frag_coord = float2(threadID.xy) + float2(0.5);
    IrradianceSpectrum irradiance = ComputeDirectIrradianceTexture(ATMOSPHERE, Params.TransmittanceLUT, Params.linearSampler, frag_coord);
    Params.IrradianceLUT[threadID.xy] = float4(irradiance, 0);
}

