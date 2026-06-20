module atmosphere_functions;

import unit_definitions;
import atmosphere_definitions;
import atmosphere_constants;
import RenderCore;

public Number ClampCosine(Number mu) {
  return clamp(mu, Number(-1.0), Number(1.0));
}

public Length ClampDistance(Length d) {
  return max(d, 0.0 * m);
}

public Length ClampRadius(AtmosphereParameters atmosphere, Length r) {
  return clamp(r, atmosphere.bottom_radius, atmosphere.top_radius);
}

public Length SafeSqrt(Area a) {
  return sqrt(max(a, 0.0 * m2));
}

public Length DistanceToTopAtmosphereBoundary(AtmosphereParameters atmosphere,
    Length r, Number mu) {
  Area discriminant = r * r * (mu * mu - 1.0) +
      atmosphere.top_radius * atmosphere.top_radius;
  return ClampDistance(-r * mu + SafeSqrt(discriminant));
}
public Length DistanceToBottomAtmosphereBoundary(AtmosphereParameters atmosphere,
    Length r, Number mu) {
  Area discriminant = r * r * (mu * mu - 1.0) +
      atmosphere.bottom_radius * atmosphere.bottom_radius;
  return ClampDistance(-r * mu - SafeSqrt(discriminant));
}
public bool RayIntersectsGround(AtmosphereParameters atmosphere,
    Length r, Number mu) {
  return mu < 0.0 && r * r * (mu * mu - 1.0) +
      atmosphere.bottom_radius * atmosphere.bottom_radius >= 0.0 * m2;
}

public Number GetLayerDensity(DensityProfileLayer layer, Length altitude) {
  Number density = layer.exp_term * exp(layer.exp_scale * altitude) +
      layer.linear_term * altitude + layer.constant_term;
  return clamp(density, Number(0.0), Number(1.0));
}

public Number GetProfileDensity(DensityProfile profile, Length altitude) {
  return altitude < profile.layers[0].width ?
      GetLayerDensity(profile.layers[0], altitude) :
      GetLayerDensity(profile.layers[1], altitude);
}

public Number GetTextureCoordFromUnitRange(Number x, int texture_size) {
  return 0.5 / Number(texture_size) + x * (1.0 - 1.0 / Number(texture_size));
}

public Number GetUnitRangeFromTextureCoord(Number u, int texture_size) {
  return (u - 0.5 / Number(texture_size)) / (1.0 - 1.0 / Number(texture_size));
}

public float2 GetTransmittanceTextureUvFromRMu(AtmosphereParameters atmosphere,
    Length r, Number mu) {
  // Distance to top atmosphere boundary for a horizontal ray at ground level.
  Length H = sqrt(atmosphere.top_radius * atmosphere.top_radius -
      atmosphere.bottom_radius * atmosphere.bottom_radius);
  // Distance to the horizon.
  Length rho =
      SafeSqrt(r * r - atmosphere.bottom_radius * atmosphere.bottom_radius);
  // Distance to the top atmosphere boundary for the ray (r,mu), and its minimum
  // and maximum values over all mu - obtained for (r,1) and (r,mu_horizon).
  Length d = DistanceToTopAtmosphereBoundary(atmosphere, r, mu);
  Length d_min = atmosphere.top_radius - r;
  Length d_max = rho + H;
  Number x_mu = (d - d_min) / (d_max - d_min);
  Number x_r = rho / H;
  return float2(GetTextureCoordFromUnitRange(x_mu, TRANSMITTANCE_TEXTURE_WIDTH),
              GetTextureCoordFromUnitRange(x_r, TRANSMITTANCE_TEXTURE_HEIGHT));
}
public void GetRMuFromTransmittanceTextureUv(AtmosphereParameters atmosphere,
    float2 uv, out Length r, out Number mu) {
  Number x_mu = GetUnitRangeFromTextureCoord(uv.x, TRANSMITTANCE_TEXTURE_WIDTH);
  Number x_r = GetUnitRangeFromTextureCoord(uv.y, TRANSMITTANCE_TEXTURE_HEIGHT);
  // Distance to top atmosphere boundary for a horizontal ray at ground level.
  Length H = sqrt(atmosphere.top_radius * atmosphere.top_radius -
      atmosphere.bottom_radius * atmosphere.bottom_radius);
  // Distance to the horizon, from which we can compute r:
  Length rho = H * x_r;
  r = sqrt(rho * rho + atmosphere.bottom_radius * atmosphere.bottom_radius);
  // Distance to the top atmosphere boundary for the ray (r,mu), and its minimum
  // and maximum values over all mu - obtained for (r,1) and (r,mu_horizon) -
  // from which we can recover mu:
  Length d_min = atmosphere.top_radius - r;
  Length d_max = rho + H;
  Length d = d_min + x_mu * (d_max - d_min);
  mu = d == 0.0 * m ? Number(1.0) : (H * H - rho * rho - d * d) / (2.0 * r * d);
  mu = ClampCosine(mu);
}


public DimensionlessSpectrum GetTransmittanceToTopAtmosphereBoundary(
    AtmosphereParameters atmosphere,
    TransmittanceTexture transmittance_texture,
    SamplerState linearSampler,
    Length r, Number mu) {
  float2 uv = GetTransmittanceTextureUvFromRMu(atmosphere, r, mu);
  return DimensionlessSpectrum(transmittance_texture.SampleLevel(linearSampler, uv, 0.0).rgb);
}

public DimensionlessSpectrum GetTransmittance(
    AtmosphereParameters atmosphere,
    TransmittanceTexture transmittance_texture,
    SamplerState linearSampler,
    Length r, Number mu, Length d, bool ray_r_mu_intersects_ground) {

  Length r_d = ClampRadius(atmosphere, sqrt(d * d + 2.0 * r * mu * d + r * r));
  Number mu_d = ClampCosine((r * mu + d) / r_d);

  if (ray_r_mu_intersects_ground) {
    return min(
        GetTransmittanceToTopAtmosphereBoundary(
            atmosphere, transmittance_texture, linearSampler, r_d, -mu_d) /
        GetTransmittanceToTopAtmosphereBoundary(
            atmosphere, transmittance_texture, linearSampler, r, -mu),
        DimensionlessSpectrum(1.0));
  } else {
    return min(
        GetTransmittanceToTopAtmosphereBoundary(
            atmosphere, transmittance_texture, linearSampler, r, mu) /
        GetTransmittanceToTopAtmosphereBoundary(
            atmosphere, transmittance_texture, linearSampler, r_d, mu_d),
        DimensionlessSpectrum(1.0));
  }
}

public DimensionlessSpectrum GetTransmittanceToSun(
    AtmosphereParameters atmosphere,
    TransmittanceTexture transmittance_texture,
    SamplerState linearSampler,
    Length r, Number mu_s) {
  Number sin_theta_h = atmosphere.bottom_radius / r;
  Number cos_theta_h = -sqrt(max(1.0 - sin_theta_h * sin_theta_h, 0.0));
  return GetTransmittanceToTopAtmosphereBoundary(
          atmosphere, transmittance_texture, linearSampler, r, mu_s) *
      smoothstep(-sin_theta_h * atmosphere.sun_angular_radius / rad,
                 sin_theta_h * atmosphere.sun_angular_radius / rad,
                 mu_s - cos_theta_h);
}

public float4 GetScatteringTextureUvwzFromRMuMuSNu(AtmosphereParameters atmosphere,
    Length r, Number mu, Number mu_s, Number nu,
    bool ray_r_mu_intersects_ground) {

  // Distance to top atmosphere boundary for a horizontal ray at ground level.
  Length H = sqrt(atmosphere.top_radius * atmosphere.top_radius -
      atmosphere.bottom_radius * atmosphere.bottom_radius);
  // Distance to the horizon.
  Length rho =
      SafeSqrt(r * r - atmosphere.bottom_radius * atmosphere.bottom_radius);
  Number u_r = GetTextureCoordFromUnitRange(rho / H, SCATTERING_TEXTURE_R_SIZE);

  // Discriminant of the quadratic equation for the intersections of the ray
  // (r,mu) with the ground (see RayIntersectsGround).
  Length r_mu = r * mu;
  Area discriminant =
      r_mu * r_mu - r * r + atmosphere.bottom_radius * atmosphere.bottom_radius;
  Number u_mu;
  if (ray_r_mu_intersects_ground) {
    // Distance to the ground for the ray (r,mu), and its minimum and maximum
    // values over all mu - obtained for (r,-1) and (r,mu_horizon).
    Length d = -r_mu - SafeSqrt(discriminant);
    Length d_min = r - atmosphere.bottom_radius;
    Length d_max = rho;
    u_mu = 0.5 - 0.5 * GetTextureCoordFromUnitRange(d_max == d_min ? 0.0 :
        (d - d_min) / (d_max - d_min), SCATTERING_TEXTURE_MU_SIZE / 2);
  } else {
    // Distance to the top atmosphere boundary for the ray (r,mu), and its
    // minimum and maximum values over all mu - obtained for (r,1) and
    // (r,mu_horizon).
    Length d = -r_mu + SafeSqrt(discriminant + H * H);
    Length d_min = atmosphere.top_radius - r;
    Length d_max = rho + H;
    u_mu = 0.5 + 0.5 * GetTextureCoordFromUnitRange(
        (d - d_min) / (d_max - d_min), SCATTERING_TEXTURE_MU_SIZE / 2);
  }

  Length d = DistanceToTopAtmosphereBoundary(
      atmosphere, atmosphere.bottom_radius, mu_s);
  Length d_min = atmosphere.top_radius - atmosphere.bottom_radius;
  Length d_max = H;
  Number a = (d - d_min) / (d_max - d_min);
  Length D = DistanceToTopAtmosphereBoundary(
      atmosphere, atmosphere.bottom_radius, atmosphere.mu_s_min);
  Number A = (D - d_min) / (d_max - d_min);
  // An ad-hoc function equal to 0 for mu_s = mu_s_min (because then d = D and
  // thus a = A), equal to 1 for mu_s = 1 (because then d = d_min and thus
  // a = 0), and with a large slope around mu_s = 0, to get more texture 
  // samples near the horizon.
  Number u_mu_s = GetTextureCoordFromUnitRange(
      max(1.0 - a / A, 0.0) / (1.0 + a), SCATTERING_TEXTURE_MU_S_SIZE);

  Number u_nu = (nu + 1.0) / 2.0;
  return float4(u_nu, u_mu_s, u_mu, u_r);
}

public void GetRMuMuSNuFromScatteringTextureUvwz(AtmosphereParameters atmosphere,
    float4 uvwz, out Length r, out Number mu, out Number mu_s,
    out Number nu, out bool ray_r_mu_intersects_ground) {

  // Distance to top atmosphere boundary for a horizontal ray at ground level.
  Length H = sqrt(atmosphere.top_radius * atmosphere.top_radius -
      atmosphere.bottom_radius * atmosphere.bottom_radius);
  // Distance to the horizon.
  Length rho =
      H * GetUnitRangeFromTextureCoord(uvwz.w, SCATTERING_TEXTURE_R_SIZE);
  r = sqrt(rho * rho + atmosphere.bottom_radius * atmosphere.bottom_radius);

  if (uvwz.z < 0.5) {
    // Distance to the ground for the ray (r,mu), and its minimum and maximum
    // values over all mu - obtained for (r,-1) and (r,mu_horizon) - from which
    // we can recover mu:
    Length d_min = r - atmosphere.bottom_radius;
    Length d_max = rho;
    Length d = d_min + (d_max - d_min) * GetUnitRangeFromTextureCoord(
        1.0 - 2.0 * uvwz.z, SCATTERING_TEXTURE_MU_SIZE / 2);
    mu = d == 0.0 * m ? Number(-1.0) :
        ClampCosine(-(rho * rho + d * d) / (2.0 * r * d));
    ray_r_mu_intersects_ground = true;
  } else {
    // Distance to the top atmosphere boundary for the ray (r,mu), and its
    // minimum and maximum values over all mu - obtained for (r,1) and
    // (r,mu_horizon) - from which we can recover mu:
    Length d_min = atmosphere.top_radius - r;
    Length d_max = rho + H;
    Length d = d_min + (d_max - d_min) * GetUnitRangeFromTextureCoord(
        2.0 * uvwz.z - 1.0, SCATTERING_TEXTURE_MU_SIZE / 2);
    mu = d == 0.0 * m ? Number(1.0) :
        ClampCosine((H * H - rho * rho - d * d) / (2.0 * r * d));
    ray_r_mu_intersects_ground = false;
  }

  Number x_mu_s =
      GetUnitRangeFromTextureCoord(uvwz.y, SCATTERING_TEXTURE_MU_S_SIZE);
  Length d_min = atmosphere.top_radius - atmosphere.bottom_radius;
  Length d_max = H;
  Length D = DistanceToTopAtmosphereBoundary(
      atmosphere, atmosphere.bottom_radius, atmosphere.mu_s_min);
  Number A = (D - d_min) / (d_max - d_min);
  Number a = (A - x_mu_s * A) / (1.0 + x_mu_s * A);
  Length d = d_min + min(a, A) * (d_max - d_min);
  mu_s = d == 0.0 * m ? Number(1.0) :
     ClampCosine((H * H - d * d) / (2.0 * atmosphere.bottom_radius * d));

  nu = ClampCosine(uvwz.x * 2.0 - 1.0);
}

public void GetRMuMuSNuFromScatteringTextureFragCoord(
    AtmosphereParameters atmosphere, float3 frag_coord,
    out Length r, out Number mu, out Number mu_s, out Number nu,
    out bool ray_r_mu_intersects_ground) {
  static const float4 SCATTERING_TEXTURE_SIZE = float4(
      SCATTERING_TEXTURE_NU_SIZE - 1,
      SCATTERING_TEXTURE_MU_S_SIZE,
      SCATTERING_TEXTURE_MU_SIZE,
      SCATTERING_TEXTURE_R_SIZE);
  Number frag_coord_nu =
      floor(frag_coord.x / Number(SCATTERING_TEXTURE_MU_S_SIZE));
  Number frag_coord_mu_s =
      fmod(frag_coord.x, Number(SCATTERING_TEXTURE_MU_S_SIZE));
  float4 uvwz =
      float4(frag_coord_nu, frag_coord_mu_s, frag_coord.y, frag_coord.z) /
          SCATTERING_TEXTURE_SIZE;
  GetRMuMuSNuFromScatteringTextureUvwz(
      atmosphere, uvwz, r, mu, mu_s, nu, ray_r_mu_intersects_ground);
  // Clamp nu to its valid range of values, given mu and mu_s.
  nu = clamp(nu, mu * mu_s - sqrt((1.0 - mu * mu) * (1.0 - mu_s * mu_s)),
      mu * mu_s + sqrt((1.0 - mu * mu) * (1.0 - mu_s * mu_s)));
}

public InverseSolidAngle RayleighPhaseFunction(Number nu) {
  InverseSolidAngle k = 3.0 / (16.0 * PI * sr);
  return k * (1.0 + nu * nu);
}

public InverseSolidAngle MiePhaseFunction(Number g, Number nu) {
  InverseSolidAngle k = 3.0 / (8.0 * PI * sr) * (1.0 - g * g) / (2.0 + g * g);
  return k * (1.0 + nu * nu) / pow(1.0 + g * g - 2.0 * g * nu, 1.5);
}

public AbstractSpectrum GetScattering(
    AtmosphereParameters atmosphere,
    AbstractScatteringTexture scattering_texture,
    SamplerState linearSampler,
    Length r, Number mu, Number mu_s, Number nu,
    bool ray_r_mu_intersects_ground) {
  float4 uvwz = GetScatteringTextureUvwzFromRMuMuSNu(
      atmosphere, r, mu, mu_s, nu, ray_r_mu_intersects_ground);
  Number tex_coord_x = uvwz.x * Number(SCATTERING_TEXTURE_NU_SIZE - 1);
  Number tex_x = floor(tex_coord_x);
  Number lerp = tex_coord_x - tex_x;
  float3 uvw0 = float3((tex_x + uvwz.y) / Number(SCATTERING_TEXTURE_NU_SIZE),
      uvwz.z, uvwz.w);
  float3 uvw1 = float3((tex_x + 1.0 + uvwz.y) / Number(SCATTERING_TEXTURE_NU_SIZE),
      uvwz.z, uvwz.w);
  return AbstractSpectrum(scattering_texture.SampleLevel(linearSampler, uvw0, 0.0).rgb * (1.0 - lerp) +
      scattering_texture.SampleLevel(linearSampler, uvw1, 0.0).rgb * lerp);
}

// Same lookup as GetScattering but keeps the alpha channel, which in the
// combined-scattering convention stores the red component of single Mie
// scattering (used to reconstruct full Mie scattering at runtime).
public float4 GetScattering4(
    AtmosphereParameters atmosphere,
    AbstractScatteringTexture scattering_texture,
    SamplerState linearSampler,
    Length r, Number mu, Number mu_s, Number nu,
    bool ray_r_mu_intersects_ground) {
  float4 uvwz = GetScatteringTextureUvwzFromRMuMuSNu(
      atmosphere, r, mu, mu_s, nu, ray_r_mu_intersects_ground);
  Number tex_coord_x = uvwz.x * Number(SCATTERING_TEXTURE_NU_SIZE - 1);
  Number tex_x = floor(tex_coord_x);
  Number lerp = tex_coord_x - tex_x;
  float3 uvw0 = float3((tex_x + uvwz.y) / Number(SCATTERING_TEXTURE_NU_SIZE),
      uvwz.z, uvwz.w);
  float3 uvw1 = float3((tex_x + 1.0 + uvwz.y) / Number(SCATTERING_TEXTURE_NU_SIZE),
      uvwz.z, uvwz.w);
  return scattering_texture.SampleLevel(linearSampler, uvw0, 0.0) * (1.0 - lerp) +
      scattering_texture.SampleLevel(linearSampler, uvw1, 0.0) * lerp;
}

// Reconstructs single Mie scattering from the combined scattering value, where
// rgb holds Rayleigh single scattering and alpha holds the red component of Mie
// single scattering. See Bruneton's "Precomputed Atmospheric Scattering".
public IrradianceSpectrum GetExtrapolatedSingleMieScattering(
    AtmosphereParameters atmosphere, float4 scattering) {
  if (scattering.r <= 0.0) {
    return IrradianceSpectrum(0.0);
  }
  return scattering.rgb * scattering.a / scattering.r *
      (atmosphere.rayleigh_scattering.r / atmosphere.mie_scattering.r) *
      (atmosphere.mie_scattering / atmosphere.rayleigh_scattering);
}

public RadianceSpectrum GetScattering(
    AtmosphereParameters atmosphere,
    ReducedScatteringTexture single_rayleigh_scattering_texture,
    ReducedScatteringTexture single_mie_scattering_texture,
    ScatteringTexture multiple_scattering_texture,
    SamplerState linearSampler,
    Length r, Number mu, Number mu_s, Number nu,
    bool ray_r_mu_intersects_ground,
    int scattering_order) {
  if (scattering_order == 1) {
    IrradianceSpectrum rayleigh = GetScattering(
        atmosphere, single_rayleigh_scattering_texture, linearSampler, r, mu, mu_s, nu,
        ray_r_mu_intersects_ground);
    IrradianceSpectrum mie = GetScattering(
        atmosphere, single_mie_scattering_texture, linearSampler, r, mu, mu_s, nu,
        ray_r_mu_intersects_ground);
    return rayleigh * RayleighPhaseFunction(nu) +
        mie * MiePhaseFunction(atmosphere.mie_phase_function_g, nu);
  } else {
    return GetScattering(
        atmosphere, multiple_scattering_texture, linearSampler, r, mu, mu_s, nu,
        ray_r_mu_intersects_ground);
  }
}

public float2 GetIrradianceTextureUvFromRMuS(AtmosphereParameters atmosphere,
    Length r, Number mu_s) {
  Number x_r = (r - atmosphere.bottom_radius) /
      (atmosphere.top_radius - atmosphere.bottom_radius);
  Number x_mu_s = mu_s * 0.5 + 0.5;
  return float2(GetTextureCoordFromUnitRange(x_mu_s, IRRADIANCE_TEXTURE_WIDTH),
              GetTextureCoordFromUnitRange(x_r, IRRADIANCE_TEXTURE_HEIGHT));
}

public void GetRMuSFromIrradianceTextureUv(AtmosphereParameters atmosphere,
    float2 uv, out Length r, out Number mu_s) {
  Number x_mu_s = GetUnitRangeFromTextureCoord(uv.x, IRRADIANCE_TEXTURE_WIDTH);
  Number x_r = GetUnitRangeFromTextureCoord(uv.y, IRRADIANCE_TEXTURE_HEIGHT);
  r = atmosphere.bottom_radius +
      x_r * (atmosphere.top_radius - atmosphere.bottom_radius);
  mu_s = ClampCosine(2.0 * x_mu_s - 1.0);
}

public static const float2 IRRADIANCE_TEXTURE_SIZE =
    float2(IRRADIANCE_TEXTURE_WIDTH, IRRADIANCE_TEXTURE_HEIGHT);
    

public IrradianceSpectrum GetIrradiance(
    AtmosphereParameters atmosphere,
    IrradianceTexture irradiance_texture,
    SamplerState linearSampler,
    Length r, Number mu_s) {
  float2 uv = GetIrradianceTextureUvFromRMuS(atmosphere, r, mu_s);
  return IrradianceSpectrum(irradiance_texture.SampleLevel(linearSampler, uv, 0.0).rgb);
}

public Length DistanceToNearestAtmosphereBoundary(AtmosphereParameters atmosphere,
    Length r, Number mu, bool ray_r_mu_intersects_ground) {
  if (ray_r_mu_intersects_ground) {
    return DistanceToBottomAtmosphereBoundary(atmosphere, r, mu);
  } else {
    return DistanceToTopAtmosphereBoundary(atmosphere, r, mu);
  }
}

public float2 GetUVFromViewVector(float3 v)
{
	return v.xy / 2 + float2(0.5);
}

public float3 GetViewVectorFromUV(float2 uv)
{
	float2 xy = (uv - float2(0.5)) * 2;
	float length2 = dot(xy, xy);
	float z = sqrt(max(1 - length2, 0.0));
	return normalize(float3(xy, z));
}

public IrradianceSpectrum GetCombinedScattering(
    AtmosphereParameters atmosphere,
    ReducedScatteringTexture scattering_texture,
    ReducedScatteringTexture single_mie_scattering_texture,
    SamplerState linearSampler,
    Length r, Number mu, Number mu_s, Number nu,
    bool ray_r_mu_intersects_ground,
    out IrradianceSpectrum single_mie_scattering) {
  float4 uvwz = GetScatteringTextureUvwzFromRMuMuSNu(
      atmosphere, r, mu, mu_s, nu, ray_r_mu_intersects_ground);
  Number tex_coord_x = uvwz.x * Number(SCATTERING_TEXTURE_NU_SIZE - 1);
  Number tex_x = floor(tex_coord_x);
  Number lerp = tex_coord_x - tex_x;
  float3 uvw0 = float3((tex_x + uvwz.y) / Number(SCATTERING_TEXTURE_NU_SIZE),
      uvwz.z, uvwz.w);
  float3 uvw1 = float3((tex_x + 1.0 + uvwz.y) / Number(SCATTERING_TEXTURE_NU_SIZE),
      uvwz.z, uvwz.w);

  IrradianceSpectrum scattering = IrradianceSpectrum(
      scattering_texture.SampleLevel(linearSampler, uvw0, 0.0).rgb * (1.0 - lerp) +
      scattering_texture.SampleLevel(linearSampler, uvw1, 0.0).rgb * lerp);
  single_mie_scattering = IrradianceSpectrum(
      single_mie_scattering_texture.SampleLevel(linearSampler, uvw0, 0.0).rgb * (1.0 - lerp) +
      single_mie_scattering_texture.SampleLevel(linearSampler, uvw1, 0.0).rgb * lerp);

  return scattering;
}

public RadianceSpectrum GetSkyRadiance(
    AtmosphereParameters atmosphere,
    TransmittanceTexture transmittance_texture,
    ReducedScatteringTexture scattering_texture,
    ReducedScatteringTexture single_mie_scattering_texture,
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
        atmosphere, scattering_texture, single_mie_scattering_texture,
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
        atmosphere, scattering_texture, single_mie_scattering_texture,
        linearSampler, r_p, mu_p, mu_s_p, nu, ray_r_mu_intersects_ground,
        single_mie_scattering);
    DimensionlessSpectrum shadow_transmittance =
        GetTransmittance(atmosphere, transmittance_texture, linearSampler,
            r, mu, shadow_length, ray_r_mu_intersects_ground);
    scattering = scattering * shadow_transmittance;
    single_mie_scattering = single_mie_scattering * shadow_transmittance;
  }
  return scattering * RayleighPhaseFunction(nu) + single_mie_scattering *
      MiePhaseFunction(atmosphere.mie_phase_function_g, nu);
}

public RadianceSpectrum GetSkyRadianceToPoint(
    AtmosphereParameters atmosphere,
    TransmittanceTexture transmittance_texture,
    ReducedScatteringTexture scattering_texture,
    ReducedScatteringTexture single_mie_scattering_texture,
    SamplerState linearSampler,
    Position camera, Position point, Length shadow_length,
    Direction sun_direction, out DimensionlessSpectrum transmittance) {
  // Compute the distance to the top atmosphere boundary along the view ray,
  // assuming the viewer is in space (or NaN if the view ray does not intersect
  // the atmosphere).
  Direction view_ray = normalize(point - camera);
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
  }

  // Compute the r, mu, mu_s and nu parameters for the first texture lookup.
  Number mu = rmu / r;
  Number mu_s = dot(camera, sun_direction) / r;
  Number nu = dot(view_ray, sun_direction);
  Length d = length(point - camera);
  bool ray_r_mu_intersects_ground = RayIntersectsGround(atmosphere, r, mu);

  transmittance = GetTransmittance(atmosphere, transmittance_texture, linearSampler,
      r, mu, d, ray_r_mu_intersects_ground);

  IrradianceSpectrum single_mie_scattering;
  IrradianceSpectrum scattering = GetCombinedScattering(
      atmosphere, scattering_texture, single_mie_scattering_texture,
      linearSampler, r, mu, mu_s, nu, ray_r_mu_intersects_ground,
      single_mie_scattering);

  // Compute the r, mu, mu_s and nu parameters for the second texture lookup.
  // If shadow_length is not 0 (case of light shafts), we want to ignore the
  // scattering along the last shadow_length meters of the view ray, which we
  // do by subtracting shadow_length from d (this way scattering_p is equal to
  // the S|x_s=x_0-lv term in Eq. (17) of our paper).
  d = max(d - shadow_length, 0.0 * m);
  Length r_p = ClampRadius(atmosphere, sqrt(d * d + 2.0 * r * mu * d + r * r));
  Number mu_p = (r * mu + d) / r_p;
  Number mu_s_p = (r * mu_s + d * nu) / r_p;

  IrradianceSpectrum single_mie_scattering_p;
  IrradianceSpectrum scattering_p = GetCombinedScattering(
      atmosphere, scattering_texture, single_mie_scattering_texture,
      linearSampler, r_p, mu_p, mu_s_p, nu, ray_r_mu_intersects_ground,
      single_mie_scattering_p);

  // Combine the lookup results to get the scattering between camera and point.
  DimensionlessSpectrum shadow_transmittance = transmittance;
  if (shadow_length > 0.0 * m) {
    // This is the T(x,x_s) term in Eq. (17) of our paper, for light shafts.
    shadow_transmittance = GetTransmittance(atmosphere, transmittance_texture, linearSampler,
        r, mu, d, ray_r_mu_intersects_ground);
  }
  scattering = scattering - shadow_transmittance * scattering_p;
  single_mie_scattering =
      single_mie_scattering - shadow_transmittance * single_mie_scattering_p;

  // Hack to avoid rendering artifacts when the sun is below the horizon.
  single_mie_scattering = single_mie_scattering *
      smoothstep(Number(0.0), Number(0.01), mu_s);

  return scattering * RayleighPhaseFunction(nu) + single_mie_scattering *
      MiePhaseFunction(atmosphere.mie_phase_function_g, nu);
}
public RadianceSpectrum GetSolarRadiance(AtmosphereParameters atmosphere) {
    return atmosphere.solar_irradiance /
        (PI * atmosphere.sun_angular_radius * atmosphere.sun_angular_radius);
}

public float3 hdr(float3 L, float hdrExposure) {
//    float hdrExposure = 12;
//    L = L * hdrExposure;
//    L.r = L.r < 1.413 ? pow(L.r * 0.38317, 1.0 / 2.2) : 1.0 - exp(-L.r);
//    L.g = L.g < 1.413 ? pow(L.g * 0.38317, 1.0 / 2.2) : 1.0 - exp(-L.g);
//    L.b = L.b < 1.413 ? pow(L.b * 0.38317, 1.0 / 2.2) : 1.0 - exp(-L.b);
//    return L;
    float3 mapped = float3(1.0) - exp(-L * hdrExposure);
    // Gamma校正
    mapped = pow(mapped, float3(1.0 / 2.2));

    return mapped;
}

public float3 GetSkyColor(    
    AtmosphereParameters atmosphere,
    TransmittanceTexture transmittance_texture,
    ReducedScatteringTexture scattering_texture,
    ReducedScatteringTexture single_mie_scattering_texture,
    SamplerState linearSampler,
    Length r, Direction view_ray, Length shadow_length,
    Direction sun_direction, Angle sun_angular_radius)
{
    float3 camera = float3(0, 0, r);
    DimensionlessSpectrum transmittance;
    float3 color = GetSkyRadiance(atmosphere, transmittance_texture, scattering_texture, single_mie_scattering_texture, linearSampler, camera, view_ray, shadow_length, sun_direction, transmittance);
	if (dot(sun_direction, view_ray) > cos(sun_angular_radius))
	{
        color += transmittance * atmosphere.solar_irradiance;
	}
    return color;
}

