// 浮游生物的散射相位函数
float Phase_p(float cosTheta)
{
    return 0.25 / PI;
}
// Henyey-Greenstein phase function
float HG_Phase(float cosTheta, float g)
{
    float g2 = g*g;
    return (1-g2) / (4 * PI * pow((1 + g2 - 2 * g * cosTheta), 1.5));
}
// 矿物的散射相位函数
float Phase_d(WaterbodyParameters waterbody, float cosTheta)
{
    return waterbody.zeta * HG_Phase(cosTheta, waterbody.gf) + (1-waterbody.zeta) * HG_Phase(cosTheta, waterbody.gb);
}
// CDOM的散射相位函数
float Phase_y(float cosTheta)
{
    return 0.25 / PI;
}
// 纯水的散射相位函数
float Phase_w(float cosTheta)
{
    return 0.06225 * (1 + 0.835 * cosTheta*cosTheta);
}
vec3 Transmittance(WaterbodyParameters waterbody, vec3 x0, vec3 x)
{
    return exp(-waterbody.total_extinction * waterbody.fog_density * length(x0-x));
}
vec3 Phase(WaterbodyParameters waterbody, float cosTheta)
{
    return 
        (waterbody.pure_water_scattering * Phase_w(cosTheta) + 
        waterbody.minerals_scattering * Phase_d(waterbody, cosTheta) + 
        waterbody.phytoplankton_scattering * Phase_p(cosTheta)) / 
        waterbody.total_scattering;
}

Number GetTextureCoordFromUnitRange_Water(Number x, int texture_size) {
  return 0.5 / Number(texture_size) + x * (1.0 - 1.0 / Number(texture_size));
}

Number GetUnitRangeFromTextureCoord_Water(Number u, int texture_size) {
  // 0.5 / Number(texture_size)是为了表示空出半个像素，1.0 / Number(texture_size)是因为两边各有半个像素是空出来的
  return (u - 0.5 / Number(texture_size)) / (1.0 - 1.0 / Number(texture_size));
}

vec4 GetScatteringTextureXYZWFromRMuMuSNu(
    WaterbodyParameters waterbody,
    Length r, Number mu, Number mu_s, Number nu,
	bool ray_r_mu_intersects_ground)
{
	vec4 XYZW = vec4(0);
	XYZW.x =  GetTextureCoordFromUnitRange_Water(nu / radians(180), WATER_SCATTERING_TEXTURE_NU_SIZE);
	XYZW.y =  GetTextureCoordFromUnitRange_Water(mu_s / radians(48.6), WATER_SCATTERING_TEXTURE_MU_S_SIZE);
	XYZW.z =  GetTextureCoordFromUnitRange_Water(mu / radians(180), WATER_SCATTERING_TEXTURE_MU_SIZE);
	XYZW.w =  GetTextureCoordFromUnitRange_Water(r / waterbody.water_depth, WATER_SCATTERING_TEXTURE_R_SIZE);
	return XYZW;
}


vec3 GetScattering(
    WaterbodyParameters waterbody,
    sampler3D scattering_texture,
    Length r, Number mu, Number mu_s, Number nu,
    bool ray_r_mu_intersects_ground) {
  vec4 xyzw = GetScatteringTextureXYZWFromRMuMuSNu(
      waterbody, r, mu, mu_s, nu, ray_r_mu_intersects_ground);
  Number tex_coord_x = xyzw.x * Number(WATER_SCATTERING_TEXTURE_NU_SIZE);
  Number tex_x = floor(tex_coord_x);
  Number lerp = tex_coord_x - tex_x;
  vec3 xyzw0 = vec3((tex_x + xyzw.y) / Number(WATER_SCATTERING_TEXTURE_NU_SIZE),
      xyzw.z, xyzw.w);
  vec3 xyzw1 = vec3((tex_x + 1.0 + xyzw.y) / Number(WATER_SCATTERING_TEXTURE_NU_SIZE),
      xyzw.z, xyzw.w);
  return vec3(texture(scattering_texture, xyzw0) * (1.0 - lerp) +
      texture(scattering_texture, xyzw1) * lerp);
}
/*
RadianceSpectrum GetScattering(
    WaterbodyParameters waterbody,
    ReducedScatteringTexture single_scattering_texture,
    Length r, Number mu, Number mu_s, Number nu,
    bool ray_r_mu_intersects_ground,
    int scattering_order) {
//  if (scattering_order == 1) {
    IrradianceSpectrum scattering = GetScattering(
        waterbody, single_scattering_texture, r, mu, mu_s, nu,
        ray_r_mu_intersects_ground);
    return scattering * Phase(waterbody, cos(nu));
//  } else {
//    return GetScattering(
//        atmosphere, multiple_scattering_texture, r, mu, mu_s, nu,
//        ray_r_mu_intersects_ground);
//  }
}
*/
void GetRMuMuSNuFromScatteringTextureXYZW(WaterbodyParameters waterbody,
    vec4 XYZW, out Length r, out Angle mu, out Angle mu_s,
    out Angle nu, out bool ray_r_mu_intersects_ground) {
    
    nu = radians(180.0) * GetUnitRangeFromTextureCoord_Water(XYZW.x, WATER_SCATTERING_TEXTURE_NU_SIZE);
    mu_s = radians(48.6) * GetUnitRangeFromTextureCoord_Water(XYZW.y, WATER_SCATTERING_TEXTURE_MU_S_SIZE);
    mu = radians(180.0) * GetUnitRangeFromTextureCoord_Water(XYZW.z, WATER_SCATTERING_TEXTURE_MU_SIZE);
    r = waterbody.water_depth * GetUnitRangeFromTextureCoord_Water(XYZW.w, WATER_SCATTERING_TEXTURE_R_SIZE);
    if (mu > radians(90.f))
        ray_r_mu_intersects_ground = true;
    else
        ray_r_mu_intersects_ground = false;
//    if (UVWZ.z < 0.5) {
//        // Distance to the ground for the ray (r,mu), and its minimum and maximum
//        // values over all mu - obtained for (r,-1) and (r,mu_horizon) - from which
//        // we can recover mu:
//        Length d_min = r - waterbody.bottom_radius;
//        Length d_max = rho;
//        Length d = d_min + (d_max - d_min) * GetUnitRangeFromTextureCoord(
//            1.0 - 2.0 * UVWZ.z, SCATTERING_TEXTURE_MU_SIZE / 2);
//        mu = d == 0.0 * m ? Number(-1.0) :
//            ClampCosine(-(rho * rho + d * d) / (2.0 * r * d));
//        ray_r_mu_intersects_ground = true;
//    } else {
//        // Distance to the top waterbody boundary for the ray (r,mu), and its
//        // minimum and maximum values over all mu - obtained for (r,1) and
//        // (r,mu_horizon) - from which we can recover mu:
//        Length d_min = waterbody.top_radius - r;
//        Length d_max = rho + H;
//        Length d = d_min + (d_max - d_min) * GetUnitRangeFromTextureCoord(
//            2.0 * UVWZ.z - 1.0, SCATTERING_TEXTURE_MU_SIZE / 2);
//        mu = d == 0.0 * m ? Number(1.0) :
//            ClampCosine((H * H - rho * rho - d * d) / (2.0 * r * d));
//        ray_r_mu_intersects_ground = false;
//    }

}
void GetRMuMuSNuFromScatteringTextureFragCoord(
    WaterbodyParameters waterbody, vec3 frag_coord,
    out Length r, out Angle mu, out Angle mu_s, out Angle nu,
    out bool ray_r_mu_intersects_ground) {
  const vec4 SCATTERING_TEXTURE_SIZE = vec4(
      WATER_SCATTERING_TEXTURE_NU_SIZE,
      WATER_SCATTERING_TEXTURE_MU_S_SIZE,
      WATER_SCATTERING_TEXTURE_MU_SIZE,
      WATER_SCATTERING_TEXTURE_R_SIZE);
  Number frag_coord_nu =
      floor(frag_coord.x / Number(WATER_SCATTERING_TEXTURE_MU_S_SIZE)) + 0.5;
  Number frag_coord_mu_s =
      mod(frag_coord.x, Number(WATER_SCATTERING_TEXTURE_MU_S_SIZE));
  vec4 xyzw =
      vec4(frag_coord_nu, frag_coord_mu_s, frag_coord.y, frag_coord.z) /
          SCATTERING_TEXTURE_SIZE;
  GetRMuMuSNuFromScatteringTextureXYZW(
      waterbody, xyzw, r, mu, mu_s, nu, ray_r_mu_intersects_ground);

  nu = clamp(nu, abs(mu - mu_s), mu + mu_s);
}

Length DistanceToBottomWaterbodyBoundary(WaterbodyParameters waterbody,
    Length r, Angle mu) {
    return clamp(r / -cos(mu), 0, MAX_WATER_OPTICAL_DEPTH);
}
Length DistanceToTopWaterbodyBoundary(WaterbodyParameters waterbody,
    Length r, Angle mu) {
    return clamp((waterbody.water_depth - r) / cos(mu), 0, MAX_WATER_OPTICAL_DEPTH);
}
Length DistanceToNearestWaterbodyBoundary(WaterbodyParameters waterbody,
    Length r, Angle mu, bool ray_r_mu_intersects_bottom)
{
  if (ray_r_mu_intersects_bottom) {
    return DistanceToBottomWaterbodyBoundary(waterbody, r, mu);
  } else {
    return DistanceToTopWaterbodyBoundary(waterbody, r, mu);
  }
}
vec3 GetTransmittance(WaterbodyParameters waterbody, Length d)
{
    return min(
        exp(-waterbody.total_extinction * waterbody.fog_density *d),
        DimensionlessSpectrum(1.0));
}

vec3 GetTransmittanceToWaterSurface(WaterbodyParameters waterbody, Length r, Angle mu)
{
    return GetTransmittance(waterbody, DistanceToTopWaterbodyBoundary(waterbody, r, mu));
}