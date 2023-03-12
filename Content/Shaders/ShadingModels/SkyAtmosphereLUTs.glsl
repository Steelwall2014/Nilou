#ifndef SKT_ATMOSPHERE_LUT_H
#define SKT_ATMOSPHERE_LUT_H

vec3 earth_center = vec3(0, 0, -ATMOSPHERE.bottom_radius);
uniform sampler2D TransmittanceLUT;
uniform sampler3D ScatteringRayleighLUT;
uniform sampler3D ScatteringMieLUT;

#endif