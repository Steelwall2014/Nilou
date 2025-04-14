#ifndef MACROS_H
#define MACROS_H

#define MAT_BINDING(binding_point) binding=binding_point
#define VF_BINDING(binding_point) binding=16+binding_point
#define MS_VS_BINDING(binding_point) binding=32+binding_point
#define MS_PS_BINDING(binding_point) binding=16+binding_point
#define GS_BINDING(binding_point) binding=binding_point

#define VERTEX_SHADER_SET_INDEX 0
#define PIXEL_SHADER_SET_INDEX 1
#define VERTEX_FACTORY_SET_INDEX 2
#define MATERIAL_SET_INDEX 3

#if RHI_API == RHI_OPENGL
	#define gl_InstanceIndex gl_InstanceID
#endif 
#if RHI_API == RHI_OPENGL
	#define gl_VertexIndex gl_VertexID
#endif 

#endif