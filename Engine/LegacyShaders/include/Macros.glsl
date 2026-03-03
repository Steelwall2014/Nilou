#ifndef MACROS_H
#define MACROS_H

#if RHI_API == RHI_OPENGL
	#define gl_InstanceIndex gl_InstanceID
#endif 
#if RHI_API == RHI_OPENGL
	#define gl_VertexIndex gl_VertexID
#endif 

#ifndef FOR_INTELLISENSE
#define FOR_INTELLISENSE 1
#endif

#if FOR_INTELLISENSE
#define SET_INDEX 0
#endif

#if FOR_INTELLISENSE
#define BINDING_INDEX 0
#endif

#endif