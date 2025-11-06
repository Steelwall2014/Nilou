#pragma once
#include <memory>
#include <string>
#include <map>
#include <string_view>
#include <type_traits>
// #include <vcruntime.h>
#include <vector>

#include "NObject/Object.h"
#include "Containers/Array.h"
#include "HAL/Platform.h"
#include "RHIDefinitions.h"
#include "RenderResource.h"
#include "Math/Maths.h"
#include "DynamicRHI.h"
#include "RenderGraph.h"

    
/** Alignment of the shader parameters struct is required to be 16-byte boundaries. */
#define SHADER_PARAMETER_STRUCT_ALIGNMENT 16

/** The alignment in bytes between elements of array shader parameters. */
#define SHADER_PARAMETER_ARRAY_ELEMENT_ALIGNMENT 16

namespace nilou {

    // /** Alignements tools because alignas() does not work on type in clang. */
    // template<typename T, int32 Alignment>
    // class TAlignedTypedef;

    // #define IMPLEMENT_ALIGNED_TYPE(Alignment) \
    //     template<typename T> \
    //     class alignas(Alignment) TAlignedTypedef<T,Alignment> \
    //     { \
    //     public: \
    //         T Data; \
    //         void operator=(const T &Other) \ 
    //         { \
    //             Data = Other; \
    //         } \
    //     };

    // IMPLEMENT_ALIGNED_TYPE(1);
    // IMPLEMENT_ALIGNED_TYPE(2);
    // IMPLEMENT_ALIGNED_TYPE(4);
    // IMPLEMENT_ALIGNED_TYPE(8);
    // IMPLEMENT_ALIGNED_TYPE(16);
    // #undef IMPLEMENT_ALIGNED_TYPE

    /** The base type of a value in a shader parameter structure. */
    enum EUniformBufferBaseType : uint8
    {
        UBMT_INVALID,

        // Invalid type when trying to use bool, to have explicit error message to programmer on why
        // they shouldn't use bool in shader parameter structures.
        UBMT_BOOL,

        // Parameter types.
        UBMT_INT32,
        UBMT_UINT32,
        UBMT_FLOAT32,
        UBMT_FLOAT64,

        // Nested structure.
        UBMT_NESTED_STRUCT,

        EUniformBufferBaseType_Num,
    };

    template<typename TypeParameter>
    struct TShaderParameterTypeInfo
    {
        static constexpr EUniformBufferBaseType BaseType = UBMT_INVALID;
        static constexpr int32 NumRows = 1;
        static constexpr int32 NumColumns = 1;
        static constexpr int32 NumElements = 0;
	    static constexpr int32 Alignment = 0;

	    // using TAlignedType = TypeParameter;
        static_assert(std::is_same<TypeParameter, bool>::value != true, "Boolean type for uniform buffer is not supported, you have to use int32 in cpp and bool in glsl");
    };

    // template<>
    // struct TShaderParameterTypeInfo<bool>
    // {
    //     static constexpr EUniformBufferBaseType BaseType = EUniformBufferBaseType::UBMT_bool;
    //     static constexpr int32 NumRows = 1;
    //     static constexpr int32 NumColumns = 1;
    //     static constexpr int32 NumElements = 0;
    //     static constexpr int32 Alignment = 4;
	
	//     // using TAlignedType = TAlignedTypedef<int32, Alignment>;
    // };

    template<>
    struct TShaderParameterTypeInfo<uint32>
    {
        static constexpr EUniformBufferBaseType BaseType = UBMT_UINT32;
        static constexpr int32 NumRows = 1;
        static constexpr int32 NumColumns = 1;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 4;
	
	    // using TAlignedType = TAlignedTypedef<uint32, Alignment>;
    };

    template<>
    struct TShaderParameterTypeInfo<int32>
    {
        static constexpr EUniformBufferBaseType BaseType = UBMT_INT32;
        static constexpr int32 NumRows = 1;
        static constexpr int32 NumColumns = 1;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 4;
	
	    // using TAlignedType = TAlignedTypedef<int32, Alignment>;
    };

    template<>
    struct TShaderParameterTypeInfo<float>
    {
        static constexpr EUniformBufferBaseType BaseType = UBMT_FLOAT32;
        static constexpr int32 NumRows = 1;
        static constexpr int32 NumColumns = 1;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 4;
	
	    // using TAlignedType = TAlignedTypedef<float, Alignment>;
    };

    template<>
    struct TShaderParameterTypeInfo<FVector2f>
    {
        static constexpr EUniformBufferBaseType BaseType = UBMT_FLOAT32;
        static constexpr int32 NumRows = 1;
        static constexpr int32 NumColumns = 2;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 8;
	
	    // using TAlignedType = TAlignedTypedef<vec2, Alignment>;
    };

    template<>
    struct TShaderParameterTypeInfo<FVector3f>
    {
        static constexpr EUniformBufferBaseType BaseType = UBMT_FLOAT32;
        static constexpr int32 NumRows = 1;
        static constexpr int32 NumColumns = 3;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 16;
	
	    // using TAlignedType = TAlignedTypedef<vec3, Alignment>;
    };

    template<>
    struct TShaderParameterTypeInfo<FVector4f>
    {
        static constexpr EUniformBufferBaseType BaseType = UBMT_FLOAT32;
        static constexpr int32 NumRows = 1;
        static constexpr int32 NumColumns = 4;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 16;
	
	    // using TAlignedType = TAlignedTypedef<vec4, Alignment>;
    };

    template<>
    struct TShaderParameterTypeInfo<FVector2>
    {
        static constexpr EUniformBufferBaseType BaseType = UBMT_FLOAT64;
        static constexpr int32 NumRows = 1;
        static constexpr int32 NumColumns = 2;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 16;
	
	    // using TAlignedType = TAlignedTypedef<vec2, Alignment>;
    };

    template<>
    struct TShaderParameterTypeInfo<FVector>
    {
        static constexpr EUniformBufferBaseType BaseType = UBMT_FLOAT64;
        static constexpr int32 NumRows = 1;
        static constexpr int32 NumColumns = 3;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 32;
	
	    // using TAlignedType = TAlignedTypedef<vec3, Alignment>;
    };

    template<>
    struct TShaderParameterTypeInfo<FVector4>
    {
        static constexpr EUniformBufferBaseType BaseType = UBMT_FLOAT64;
        static constexpr int32 NumRows = 1;
        static constexpr int32 NumColumns = 4;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 32;
	
	    // using TAlignedType = TAlignedTypedef<vec4, Alignment>;
    };

    template<>
    struct TShaderParameterTypeInfo<FIntVector2>
    {
        static constexpr EUniformBufferBaseType BaseType = UBMT_INT32;
        static constexpr int32 NumRows = 1;
        static constexpr int32 NumColumns = 2;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 8;
	
	    // using TAlignedType = TAlignedTypedef<ivec2, Alignment>;
    };

    template<>
    struct TShaderParameterTypeInfo<FIntVector>
    {
        static constexpr EUniformBufferBaseType BaseType = UBMT_INT32;
        static constexpr int32 NumRows = 1;
        static constexpr int32 NumColumns = 3;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 16;
	
	    // using TAlignedType = TAlignedTypedef<ivec3, Alignment>;
    };

    template<>
    struct TShaderParameterTypeInfo<FIntVector4>
    {
        static constexpr EUniformBufferBaseType BaseType = UBMT_INT32;
        static constexpr int32 NumRows = 1;
        static constexpr int32 NumColumns = 4;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 16;
	
	    // using TAlignedType = TAlignedTypedef<ivec4, Alignment>;
    };

    template<>
    struct TShaderParameterTypeInfo<FUIntVector2>
    {
        static constexpr EUniformBufferBaseType BaseType = UBMT_UINT32;
        static constexpr int32 NumRows = 1;
        static constexpr int32 NumColumns = 2;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 8;
	
	    // using TAlignedType = TAlignedTypedef<uvec2, Alignment>;
    };

    template<>
    struct TShaderParameterTypeInfo<FUIntVector>
    {
        static constexpr EUniformBufferBaseType BaseType = UBMT_UINT32;
        static constexpr int32 NumRows = 1;
        static constexpr int32 NumColumns = 3;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 16;
	
	    // using TAlignedType = TAlignedTypedef<uvec3, Alignment>;
    };

    template<>
    struct TShaderParameterTypeInfo<FUIntVector4>
    {
        static constexpr EUniformBufferBaseType BaseType = UBMT_UINT32;
        static constexpr int32 NumRows = 1;
        static constexpr int32 NumColumns = 4;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 16;
	
	    // using TAlignedType = TAlignedTypedef<uvec4, Alignment>;
    };

    template<>
    struct TShaderParameterTypeInfo<FMatrix22f>
    {
        static constexpr EUniformBufferBaseType BaseType = UBMT_FLOAT32;
        static constexpr int32 NumRows = 2;
        static constexpr int32 NumColumns = 2;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 16;
	
	    // using TAlignedType = TAlignedTypedef<mat2, Alignment>;
    };

    template<>
    struct TShaderParameterTypeInfo<FMatrix33f>
    {
        static constexpr EUniformBufferBaseType BaseType = UBMT_FLOAT32;
        static constexpr int32 NumRows = 3;
        static constexpr int32 NumColumns = 3;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 16;
	
	    // using TAlignedType = TAlignedTypedef<mat3, Alignment>;
    };

    template<>
    struct TShaderParameterTypeInfo<FMatrix44f>
    {
        static constexpr EUniformBufferBaseType BaseType = UBMT_FLOAT32;
        static constexpr int32 NumRows = 4;
        static constexpr int32 NumColumns = 4;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 16;
	
	    // using TAlignedType = TAlignedTypedef<mat4, Alignment>;
    };

    template<>
    struct TShaderParameterTypeInfo<FMatrix22>
    {
        static constexpr EUniformBufferBaseType BaseType = UBMT_FLOAT64;
        static constexpr int32 NumRows = 2;
        static constexpr int32 NumColumns = 2;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 32;
	
	    // using TAlignedType = TAlignedTypedef<mat2, Alignment>;
    };

    template<>
    struct TShaderParameterTypeInfo<FMatrix33>
    {
        static constexpr EUniformBufferBaseType BaseType = UBMT_FLOAT64;
        static constexpr int32 NumRows = 3;
        static constexpr int32 NumColumns = 3;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 32;
	
	    // using TAlignedType = TAlignedTypedef<mat3, Alignment>;
    };

    template<>
    struct TShaderParameterTypeInfo<FMatrix>
    {
        static constexpr EUniformBufferBaseType BaseType = UBMT_FLOAT64;
        static constexpr int32 NumRows = 4;
        static constexpr int32 NumColumns = 4;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 32;
	
	    // using TAlignedType = TAlignedTypedef<mat4, Alignment>;
    };

    template<typename T, size_t InNumElements>
    struct TShaderParameterTypeInfo<T[InNumElements]>
    {
        static constexpr EUniformBufferBaseType BaseType = TShaderParameterTypeInfo<T>::BaseType;
        static constexpr int32 NumRows = TShaderParameterTypeInfo<T>::NumRows;
        static constexpr int32 NumColumns = TShaderParameterTypeInfo<T>::NumColumns;
        static constexpr int32 NumElements = InNumElements;
        static constexpr int32 Alignment = SHADER_PARAMETER_ARRAY_ELEMENT_ALIGNMENT;
	
	    // using TAlignedType = TAlignedStaticArray<T, InNumElements, Alignment>;
    };
}

namespace nilou {

    #define BEGIN_UNIFORM_BUFFER_STRUCT(TypeName) \
        struct TypeName \
        {
    #define SHADER_PARAMETER(Type, MemberName) \
            alignas(TShaderParameterTypeInfo<Type>::Alignment) Type MemberName;
    #define SHADER_PARAMETER_ARRAY(Type, N, MemberName) \
            TAlignedStaticArray<Type, N, TShaderParameterTypeInfo<Type[N]>::Alignment> MemberName;
    #define SHADER_PARAMETER_STRUCT(Type, MemberName) \
            Type MemberName;
    #define SHADER_PARAMETER_STRUCT_ARRAY(Type, N, MemberName) \
            std::array<Type, N> MemberName;
    #define END_UNIFORM_BUFFER_STRUCT() \
        };

}