#ifndef SKT_ATMOSPHERE_LUT_H
#define SKT_ATMOSPHERE_LUT_H

vec3 earth_center = vec3(0, 0, -ATMOSPHERE.bottom_radius);
layout (binding=51) uniform sampler2D TransmittanceLUT;
layout (binding=52) uniform sampler3D ScatteringRayleighLUT;
layout (binding=53) uniform sampler3D ScatteringMieLUT;

#endif