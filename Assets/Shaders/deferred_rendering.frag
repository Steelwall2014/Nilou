#version 430 core
#include "include/unit_definitions.glsl"
#include "atmosphere/atmosphere_definitions.glsl"
#include "atmosphere/atmosphere_functions.glsl"
#include "waterbody/waterbody_constants.glsl"
#include "waterbody/waterbody_definitions.glsl"
#include "waterbody/waterbody_functions.glsl"

//#define RAY_MARCHING;
//#define ANALYTICAL_SINGLE_SCATTER;

layout (location = 0) out vec4 FragColor;

in vec2 uv;
in vec3 cameraRay;

uniform float cameraNearClip;
uniform float cameraFarClip;

uniform sampler2D ColorBuffer;
uniform sampler2D DepthBuffer;
uniform sampler2D TransmittanceLUT;
uniform sampler3D SingleScatteringRayleighLUT;
uniform sampler3D SingleScatteringMieLUT;
uniform sampler2D OceanSurfaceHeightMap;
uniform sampler3D OceanScatteringLUT;
uniform sampler3D OceanScatteringDensityLUT;

uniform float OceanSurfaceHeightMapMeterSize;
uniform vec3 cameraPos;
uniform vec3 SunLightDir;

vec3 fogColor = vec3(1);
float RayleighScaleHeight = 8000;
float MieScaleHeight = 1200;
float RadiusEarth = ATMOSPHERE.bottom_radius * km;
float RadiusAtmosphere = ATMOSPHERE.top_radius * km;

vec3 BETA_Rayleigh = vec3(5.8, 13.5, 33.1) * 1e-6;
vec3 BETA_Mie = vec3(2.2, 2.2, 2.2) * 1e-5;
vec3 EarthCenter = vec3(cameraPos.xy, -RadiusEarth);
#include "include/functions.glsl"

vec3 GetScatteringDensity(
    WaterbodyParameters waterbody,
    sampler3D scattering_density_texture,
    Length r, Number mu, Number mu_s, Number nu) {

	vec4 xyzw = vec4(0);
	xyzw.x =  GetTextureCoordFromUnitRange_Water(nu / PI, WATER_SCATTERING_TEXTURE_NU_SIZE);
	xyzw.y =  GetTextureCoordFromUnitRange_Water(mu_s / (48.6/180.0*PI), WATER_SCATTERING_TEXTURE_MU_S_SIZE);
	xyzw.z =  GetTextureCoordFromUnitRange_Water(mu / PI, WATER_SCATTERING_TEXTURE_MU_SIZE);
	xyzw.w =  GetTextureCoordFromUnitRange_Water(r / waterbody.water_depth, WATER_SCATTERING_TEXTURE_R_SIZE);

	Number tex_coord_x = xyzw.x * Number(WATER_SCATTERING_TEXTURE_NU_SIZE);
	Number tex_x = floor(tex_coord_x);
	Number lerp = tex_coord_x - tex_x;
	vec3 xyzw0 = vec3((tex_x + xyzw.y) / Number(WATER_SCATTERING_TEXTURE_NU_SIZE),
		xyzw.z, xyzw.w);
	vec3 xyzw1 = vec3((tex_x + 1.0 + xyzw.y) / Number(WATER_SCATTERING_TEXTURE_NU_SIZE),
		xyzw.z, xyzw.w);
	return vec3(texture(scattering_density_texture, xyzw0) * (1.0 - lerp) +
		texture(scattering_density_texture, xyzw1) * lerp);
}

void main()
{
	vec2 uv2 = uv * 2.0 - 1.0;
	float depth = texture2D(DepthBuffer, uv).r;
	float z = depth * 2.0 - 1.0;
	float linearDepth = (2.0 * cameraNearClip * cameraFarClip) / (cameraFarClip + cameraNearClip - z * (cameraFarClip - cameraNearClip));
//	vec3 cameraRay2 = cameraForward * cameraNearClip + uv2.x * toRight + uv2.y * toTop;
	vec3 CameraToFrag = cameraRay / cameraNearClip * linearDepth;	// 长度不为1！！！
	vec3 CameraToFragDir = normalize(CameraToFrag);
	vec3 frag_position = cameraPos + CameraToFrag;
	float dist = length(CameraToFrag);
	float height = CameraToFrag.z;
	bool IsSky = abs(depth - 1.0f) < 1e-7;
	bool IsUnderwater = cameraPos.z < 0;//texture(OceanSurfaceHeightMap, cameraPos.xy / OceanSurfaceHeightMapMeterSize).r;
	bool IsSeabed = IsSky ? false : frag_position.z < -5.f;
	float T_PA = 0;
//	vec3 beta = vec3(0.000015);
	vec3 f = saturate(exp( - BETA_Rayleigh * dist));
//	f = (3.0f * pow(f, 2.0f) - 2.0f * pow(f, 3.0f));
	vec3 originColor = texture2D(ColorBuffer, uv).rgb;
	vec3 color = originColor;
//	if (abs(depth - 1.0f) < 1e-7)
//		color = vec3(0, 0, 0);
	vec3 sunLightDir = normalize(SunLightDir);
    vec3 sun_direction = -sunLightDir;
    vec3 camera = vec3(0, 0, cameraPos.z/km);
	if (IsUnderwater)
	{
        float fresnel = 0.02 + (1.0 - 0.02) * pow(1.0 - dot(vec3(0, 0, 1), sun_direction), 5.0);
		vec3 L = -refract(-sun_direction, vec3(0, 0, 1), 1 / 1.333);
		vec3 view_direction = CameraToFragDir;
		vec3 in_scattering = vec3(0.f);
		vec3 C = frag_position + L * (abs(frag_position.z) / L.z);

//		if (IsSky)
//		{    
//			dist = 1000;
//			frag_position = cameraPos + CameraToFragDir * dist;
//			originColor = vec3(0);
//		}
//		const int SAMPLE_NUM = 50;
//		float dx = dist / float(SAMPLE_NUM);
//		for (int i = 0; i <= SAMPLE_NUM; i += 1)
//		{
//			vec3 P = cameraPos + view_direction * dx * float(i);
//			vec3 C = P + L * (abs(P.z) / L.z);
//			vec3 transmittance = Transmittance(WATERBODY, P, cameraPos) * Transmittance(WATERBODY, C, P);
//			float cosTheta = dot(L, view_direction);
//			vec3 phase = Phase(WATERBODY, cosTheta);
//			float weight_i = (i == 0) || (i == SAMPLE_NUM) ? 0.5 : 1.f;
//			in_scattering += transmittance * phase * WATERBODY.total_scattering * dx * weight_i;
//		}
//		color = hdr(in_scattering, 1);
//		return;
////		color = Transmittance(WATERBODY, cameraPos, cameraPos + view_direction * dist) * 12;

//		/****crest*****/
//		vec3 scatterColor = vec3(0.5);
//		color = lerp(originColor, scatterColor, saturate(1.0 - exp(-0.01 * dist)));
//		/****crest*****/

		vec3 transmittance = GetTransmittance(WATERBODY, length(frag_position-cameraPos));
		if (frag_position.z < 0) 
			transmittance *= GetTransmittance(WATERBODY, length(frag_position-C));
			
		float r = WATERBODY.water_depth + cameraPos.z;
		float mu = acos(view_direction.z);
		float mu_s = acos(L.z);
		float nu = acos(dot(L, view_direction));
		int SAMPLE_NUM = 50;
		float dx = min(dist, 100) / float(SAMPLE_NUM);

#ifdef RAY_MARCHING

		float cosTheta = dot(L, view_direction);
		vec3 phase = Phase(WATERBODY, cosTheta);
		for (int i = 0; i <= SAMPLE_NUM; i += 1)
		{
			vec3 P = cameraPos + view_direction * dx * float(i);
			vec3 C = P + L * (abs(P.z) / L.z);
			float weight_i = (i == 0) || (i == SAMPLE_NUM) ? 0.5 : 1.f;
			Length d_i = Number(i) * dx;
			Length r_d = cos(mu) * d_i + r;

			vec3 singlescattering_density = GetScatteringDensity(WATERBODY, OceanScatteringDensityLUT, r_d, mu, mu_s, nu);
			in_scattering += singlescattering_density * GetTransmittance(WATERBODY, d_i) * weight_i;

//			vec3 transmittance = Transmittance(WATERBODY, P, cameraPos) * Transmittance(WATERBODY, C, P);
//			in_scattering += phase * WATERBODY.total_scattering * transmittance * weight_i;
		}
		in_scattering *= dx;

#elif defined(ANALYTICAL_SINGLE_SCATTER)

		float cos_mu = cos(mu);
		float cos_mu_s = cos(mu_s);
		vec3 a = WATERBODY.total_extinction * (cos_mu_s-cos_mu) / cos_mu_s;
		vec3 b = WATERBODY.total_extinction * (WATERBODY.water_depth-r) / cos_mu_s;
		vec3 integral = exp(-b) * (1-exp(-a * dist)) / a;
		in_scattering = integral * Phase(WATERBODY, cos(nu)) * WATERBODY.total_scattering;
#else
		in_scattering = GetScattering(WATERBODY, OceanScatteringLUT, r, mu, mu_s, nu, false);
		if (IsSeabed)
		{
			float cos_mu = cos(mu);
			float cos_mu_s = cos(mu_s);
			vec3 a = WATERBODY.total_extinction * (cos_mu_s-cos_mu) / cos_mu_s;
			float max_dist;
			if (mu < radians(90.f))
				max_dist = DistanceToTopWaterbodyBoundary(WATERBODY, r, mu);
			else
				max_dist = DistanceToBottomWaterbodyBoundary(WATERBODY, r, mu);

			vec3 ratio = max(vec3(1.f), (1 - exp(-a*dist)) / (1 - exp(-a*max_dist)));
			in_scattering *= ratio;
		}
#endif
		originColor *= transmittance;
		if (IsSeabed)
			originColor *= 100;
//		if (!IsSky)
//		{
//
//			if (IsSeabed)
//			{
//				originColor *= transmittance * 100;
//			}
//			else
//			{
//				originColor *= transmittance;
//			}
//		}
//		else
//		{	
//			originColor = vec3(0);
//		}

		color = originColor + hdr(in_scattering, WATERBODY.hdr_exposure);
		color *= (1-fresnel);
	}
	else
	{
		color = originColor;
//		if (IsSky)
//		{
//			color = vec3(0, 0, 0);
//			DimensionlessSpectrum transmittance;
//			color += GetSkyRadiance(ATMOSPHERE, TransmittanceLUT, SingleScatteringRayleighLUT, SingleScatteringMieLUT, camera - earth_center, CameraToFragDir, 0, sun_direction, transmittance);
//			if (dot(-sunLightDir, CameraToFragDir) > cos(ATMOSPHERE.sun_angular_radius))
//			{
//				color += transmittance * GetSolarRadiance();
//			}
//			color = originColor;//hdr(color, 12);
//		}
//		else
//		{
	//        vec3 point = camera + CameraToFrag / km;
	//        camera -= earth_center;
	//        point -= earth_center;
	//        vec3 transmittance;
	//        Direction view_ray = normalize(point - camera);
	//        Length r = length(camera);
	//        Length rmu = dot(camera, view_ray);
	//        Number mu = rmu / r;
	//        Number nu = dot(view_ray, sun_direction);
	//        Length d = length(point - camera);
	//        float(RayIntersectsGround(ATMOSPHERE, r, mu));
	//        vec3 in_scatter = GetSkyRadianceToPoint(ATMOSPHERE,
	//            TransmittanceLUT, SingleScatteringRayleighLUT, SingleScatteringMieLUT, 
	//            camera, point, 0, sun_direction, transmittance);
//			color = originColor/*hdr(originColor * transmittance + in_scatter, 0.40)*/;
//		}
	}

	FragColor = vec4(color, 1.0f);
//	FragColor = vec4(uv, 0.0, 1.0f);
}