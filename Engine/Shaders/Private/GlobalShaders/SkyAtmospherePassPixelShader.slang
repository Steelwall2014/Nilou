module SkyAtmospherePassPixelShader;
import RenderCore;
import "Shaders/Private/SkyAtmosphere/unit_definitions";
import "Shaders/Private/SkyAtmosphere/atmosphere_constants";
import "Shaders/Private/SkyAtmosphere/atmosphere_definitions";
import "Shaders/Private/SkyAtmosphere/atmosphere_functions";

ParameterBlock<AtmosphereParameters> ATMOSPHERE;
struct SkyAtmosphereLUTs
{
    Texture2D<float4> TransmittanceLUT;
    Texture3D<float4> ScatteringLUT;
    SamplerState linearSampler;
}
ParameterBlock<SkyAtmosphereLUTs> LUTs;

struct SkyAtmospherePassPixelShaderParameters
{
    Sampler2D<float> Depth;
    float3 SunDirection;
}
ParameterBlock<SkyAtmospherePassPixelShaderParameters> Parameters;

IrradianceSpectrum GetCombinedScattering(
    AtmosphereParameters atmosphere,
    ReducedScatteringTexture scattering_texture,
    SamplerState linearSampler,
    Length r, Number mu, Number mu_s, Number nu,
    bool ray_r_mu_intersects_ground,
    out IrradianceSpectrum single_mie_scattering) {
  float4 combined = GetScattering4(
      atmosphere, scattering_texture, linearSampler, r, mu, mu_s, nu,
      ray_r_mu_intersects_ground);
  single_mie_scattering = GetExtrapolatedSingleMieScattering(atmosphere, combined);
  return IrradianceSpectrum(combined.rgb);
}

RadianceSpectrum GetSkyRadiance(
    AtmosphereParameters atmosphere,
    TransmittanceTexture transmittance_texture,
    ReducedScatteringTexture scattering_texture,
    SamplerState linearSampler,
    Position camera, Direction view_ray, Length shadow_length,
    Direction sun_direction, out DimensionlessSpectrum transmittance) {
  // Compute the distance to the top atmosphere boundary along the view ray,
  // assuming the viewer is in space (or NaN if the view ray does not intersect
  // the atmosphere).
  Length r = length(camera);
  Length rmu = dot(camera, view_ray);
  Length distance_to_top_atmosphere_boundary = -rmu -
      sqrt(rmu * rmu - r * r + atmosphere.top_radius * atmosphere.top_radius);
  // If the viewer is in space and the view ray intersects the atmosphere, move
  // the viewer to the top atmosphere boundary (along the view ray):
  if (distance_to_top_atmosphere_boundary > 0.0 * m) {
    camera = camera + view_ray * distance_to_top_atmosphere_boundary;
    r = atmosphere.top_radius;
    rmu += distance_to_top_atmosphere_boundary;
  } else if (r > atmosphere.top_radius) {
    // If the view ray does not intersect the atmosphere, simply return 0.
    transmittance = DimensionlessSpectrum(1.0);
    return RadianceSpectrum(0.0 * watt_per_square_meter_per_sr_per_nm);
  }
  // Compute the r, mu, mu_s and nu parameters needed for the texture lookups.
  Number mu = rmu / r;
  Number mu_s = dot(camera, sun_direction) / r;
  Number nu = dot(view_ray, sun_direction);
  bool ray_r_mu_intersects_ground = RayIntersectsGround(atmosphere, r, mu);

  transmittance = ray_r_mu_intersects_ground ? DimensionlessSpectrum(0.0) :
      GetTransmittanceToTopAtmosphereBoundary(
          atmosphere, transmittance_texture, linearSampler, r, mu);
  IrradianceSpectrum single_mie_scattering;
  IrradianceSpectrum scattering;
  if (shadow_length == 0.0 * m) {
    scattering = GetCombinedScattering(
        atmosphere, scattering_texture,
        linearSampler, r, mu, mu_s, nu, ray_r_mu_intersects_ground,
        single_mie_scattering);
  } else {
    // Case of light shafts (shadow_length is the total length noted l in our
    // paper): we omit the scattering between the camera and the point at
    // distance l, by implementing Eq. (18) of the paper (shadow_transmittance
    // is the T(x,x_s) term, scattering is the S|x_s=x+lv term).
    Length d = shadow_length;
    Length r_p =
        ClampRadius(atmosphere, sqrt(d * d + 2.0 * r * mu * d + r * r));
    Number mu_p = (r * mu + d) / r_p;
    Number mu_s_p = (r * mu_s + d * nu) / r_p;

    scattering = GetCombinedScattering(
        atmosphere, scattering_texture,
        linearSampler, r_p, mu_p, mu_s_p, nu, ray_r_mu_intersects_ground,
        single_mie_scattering);
    DimensionlessSpectrum shadow_transmittance =
        GetTransmittance(atmosphere, transmittance_texture, linearSampler,
            r, mu, shadow_length, ray_r_mu_intersects_ground);
    scattering = scattering * shadow_transmittance;
    single_mie_scattering = single_mie_scattering * shadow_transmittance;
  }
  // Apply the phase functions analytically at runtime (avoids undersampling the
  // sharp Mie forward peak along the low-resolution nu axis of the LUT).
  return scattering * RayleighPhaseFunction(nu) + single_mie_scattering *
      MiePhaseFunction(atmosphere.mie_phase_function_g, nu);
}

struct PSOutput
{
    float4 SceneColor;
}

struct PSInput
{
    float4 svPosition : SV_Position;
};

[shader("pixel")]
PSOutput MainPS(PSInput input)
{
    PSOutput output;
    float2 screenUV = (input.svPosition.xy - ViewParameters.ViewRectMin) * ViewParameters.ViewSizeAndInvSize.zw;
    float depth = Parameters.Depth.SampleLevelZero(screenUV);
    if (depth != 1)
    {
        output.SceneColor = float4(0);
        return output;
    }
    Direction view_ray = GetScreenRelativeWorldDir(input.svPosition);
    float3 transmittance;
    float3 camera = ViewParameters.CameraPosition / 1000.0f + float3(0, 0, ATMOSPHERE.bottom_radius);
    float3 color = GetSkyRadiance(
        ATMOSPHERE, 
        LUTs.TransmittanceLUT, 
        LUTs.ScatteringLUT, 
        LUTs.linearSampler, 
        camera, 
        view_ray,
        0, 
        Parameters.SunDirection,
        transmittance);
    if (dot(Parameters.SunDirection, view_ray) > cos(ATMOSPHERE.sun_angular_radius))
	{
        color += transmittance * GetSolarRadiance(ATMOSPHERE);
	}
    output.SceneColor = float4(color, 1);
    return output;
}
