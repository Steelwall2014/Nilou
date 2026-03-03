module unit_definitions;

import RenderCore;

public typealias Length = float;
public typealias Wavelength = float;
public typealias Angle = float;
public typealias SolidAngle = float;
public typealias Power = float;
public typealias LuminousPower = float;

public typealias Number = float;
public typealias InverseLength = float;
public typealias Area = float;
public typealias Volume = float;
public typealias NumberDensity = float;
public typealias Irradiance = float;
public typealias Radiance = float;
public typealias SpectralPower = float;
public typealias SpectralIrradiance = float;
public typealias SpectralRadiance = float;
public typealias SpectralRadianceDensity = float;
public typealias ScatteringCoefficient = float;
public typealias InverseSolidAngle = float;
public typealias LuminousIntensity = float;
public typealias Luminance = float;
public typealias Illuminance = float;

// A generic function from Wavelength to some other type.
public typealias AbstractSpectrum = float3;
// A function from Wavelength to Number.
public typealias DimensionlessSpectrum = float3;
// A function from Wavelength to SpectralPower.
public typealias PowerSpectrum = float3;
// A function from Wavelength to SpectralIrradiance.
public typealias IrradianceSpectrum = float3;
// A function from Wavelength to SpectralRadiance.
public typealias RadianceSpectrum = float3;
// A function from Wavelength to SpectralRadianceDensity.
public typealias RadianceDensitySpectrum = float3;
// A function from Wavelength to ScaterringCoefficient.
public typealias ScatteringSpectrum = float3;

// A position in 3D (3 length values).
public typealias Position = float3;
// A unit direction vector in 3D (3 unitless values).
public typealias Direction = float3;
// A vector of 3 luminance values.
public typealias Luminance3 = float3;
// A vector of 3 illuminance values.
public typealias Illuminance3 = float3;

public typealias TransmittanceTexture = Texture2D;
public typealias AbstractScatteringTexture = Texture3D;
public typealias ReducedScatteringTexture = Texture3D;
public typealias ScatteringTexture = Texture3D;
public typealias ScatteringDensityTexture = Texture3D;
public typealias IrradianceTexture = Texture2D;

public static const Length m = 1.0;
public static const Wavelength nm = 1.0;
public static const Angle rad = 1.0;
public static const SolidAngle sr = 1.0;
public static const Power watt = 1.0;
public static const LuminousPower lm = 1.0;


public static const Length km = 1000.0 * m;
public static const Area m2 = m * m;
public static const Volume m3 = m * m * m;
public static const Angle pi = PI * rad;
public static const Angle deg = pi / 180.0;
public static const Irradiance watt_per_square_meter = watt / m2;
public static const Radiance watt_per_square_meter_per_sr = watt / (m2 * sr);
public static const SpectralIrradiance watt_per_square_meter_per_nm = watt / (m2 * nm);
public static const SpectralRadiance watt_per_square_meter_per_sr_per_nm =
    watt / (m2 * sr * nm);
public static const SpectralRadianceDensity watt_per_cubic_meter_per_sr_per_nm =
    watt / (m3 * sr * nm);
public static const LuminousIntensity cd = lm / sr;
public static const LuminousIntensity kcd = 1000.0 * cd;
public static const Luminance cd_per_square_meter = cd / m2;
public static const Luminance kcd_per_square_meter = kcd / m2;

