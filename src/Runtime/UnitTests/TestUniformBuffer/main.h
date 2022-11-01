#pragma once

#include "Common/Maths.h"
#include "glm/glm.hpp"
#include <memory>

using int32 = int;
using uint32 = unsigned int;
using uint64 = unsigned long long;
namespace nilou {

    enum class EUniformBufferBaseType
    {        
        UBMT_Null,
        UBMT_bool,
        UBMT_int,
        UBMT_uint,
        UBMT_float,
    };

    enum class EUniformBufferMemberType
    {
        UBMT_Null,
        UBMT_bool,
        UBMT_int,
        UBMT_uint,
        UBMT_float,
        UBMT_vec2,
        UBMT_vec3,
        UBMT_vec4,
        UBMT_bvec2,
        UBMT_bvec3,
        UBMT_bvec4,
        UBMT_ivec2,
        UBMT_ivec3,
        UBMT_ivec4,
        UBMT_uvec2,
        UBMT_uvec3,
        UBMT_uvec4,
        UBMT_mat2,
        UBMT_mat3,
        UBMT_mat4,
        UBMT_struct
    };

    template<typename TypeParameter>
    struct TShaderParameterTypeInfo
    {
        static constexpr EUniformBufferMemberType BaseType = EUniformBufferMemberType::UBMT_Null;
        static constexpr int32 NumRows = 1;
        static constexpr int32 NumColumns = 1;
        static constexpr int32 NumElements = 0;
	    static constexpr int32 Alignment = 0;
    };

    template<typename TypeParameter>
    TShaderParameterTypeInfo<TypeParameter> GetDummyObject() 
    { 
        static TShaderParameterTypeInfo<TypeParameter> DummyObject; 
        return DummyObject; 
    }

    template<>
    struct TShaderParameterTypeInfo<bool>
    {
        static constexpr EUniformBufferMemberType BaseType = EUniformBufferMemberType::UBMT_bool;
        static constexpr int32 NumRows = 1;
        static constexpr int32 NumColumns = 1;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 4;
    };

    template<>
    struct TShaderParameterTypeInfo<uint32>
    {
        static constexpr EUniformBufferMemberType BaseType = EUniformBufferMemberType::UBMT_uint;
        static constexpr int32 NumRows = 1;
        static constexpr int32 NumColumns = 1;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 4;
    };

    template<>
    struct TShaderParameterTypeInfo<int32>
    {
        static constexpr EUniformBufferMemberType BaseType = EUniformBufferMemberType::UBMT_int;
        static constexpr int32 NumRows = 1;
        static constexpr int32 NumColumns = 1;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 4;
    };

    template<>
    struct TShaderParameterTypeInfo<float>
    {
        static constexpr EUniformBufferMemberType BaseType = EUniformBufferMemberType::UBMT_float;
        static constexpr int32 NumRows = 1;
        static constexpr int32 NumColumns = 1;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 4;
    };

    template<>
    struct TShaderParameterTypeInfo<vec2>
    {
        static constexpr EUniformBufferMemberType BaseType = EUniformBufferMemberType::UBMT_vec2;
        static constexpr int32 NumRows = 1;
        static constexpr int32 NumColumns = 2;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 8;
    };

    template<>
    struct TShaderParameterTypeInfo<vec3>
    {
        static constexpr EUniformBufferMemberType BaseType = EUniformBufferMemberType::UBMT_vec3;
        static constexpr int32 NumRows = 1;
        static constexpr int32 NumColumns = 3;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 16;
    };

    template<>
    struct TShaderParameterTypeInfo<vec4>
    {
        static constexpr EUniformBufferMemberType BaseType = EUniformBufferMemberType::UBMT_vec4;
        static constexpr int32 NumRows = 1;
        static constexpr int32 NumColumns = 4;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 16;
    };

    template<>
    struct TShaderParameterTypeInfo<bvec2>
    {
        static constexpr EUniformBufferMemberType BaseType = EUniformBufferMemberType::UBMT_bvec2;
        static constexpr int32 NumRows = 1;
        static constexpr int32 NumColumns = 2;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 8;
    };

    template<>
    struct TShaderParameterTypeInfo<bvec3>
    {
        static constexpr EUniformBufferMemberType BaseType = EUniformBufferMemberType::UBMT_bvec3;
        static constexpr int32 NumRows = 1;
        static constexpr int32 NumColumns = 3;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 16;
    };

    template<>
    struct TShaderParameterTypeInfo<bvec4>
    {
        static constexpr EUniformBufferMemberType BaseType = EUniformBufferMemberType::UBMT_bvec4;
        static constexpr int32 NumRows = 1;
        static constexpr int32 NumColumns = 4;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 16;
    };

    template<>
    struct TShaderParameterTypeInfo<ivec2>
    {
        static constexpr EUniformBufferMemberType BaseType = EUniformBufferMemberType::UBMT_ivec2;
        static constexpr int32 NumRows = 1;
        static constexpr int32 NumColumns = 2;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 8;
    };

    template<>
    struct TShaderParameterTypeInfo<ivec3>
    {
        static constexpr EUniformBufferMemberType BaseType = EUniformBufferMemberType::UBMT_ivec3;
        static constexpr int32 NumRows = 1;
        static constexpr int32 NumColumns = 3;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 16;
    };

    template<>
    struct TShaderParameterTypeInfo<ivec4>
    {
        static constexpr EUniformBufferMemberType BaseType = EUniformBufferMemberType::UBMT_ivec4;
        static constexpr int32 NumRows = 1;
        static constexpr int32 NumColumns = 4;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 16;
    };

    template<>
    struct TShaderParameterTypeInfo<uvec2>
    {
        static constexpr EUniformBufferMemberType BaseType = EUniformBufferMemberType::UBMT_uvec2;
        static constexpr int32 NumRows = 1;
        static constexpr int32 NumColumns = 2;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 8;
    };

    template<>
    struct TShaderParameterTypeInfo<uvec3>
    {
        static constexpr EUniformBufferMemberType BaseType = EUniformBufferMemberType::UBMT_uvec3;
        static constexpr int32 NumRows = 1;
        static constexpr int32 NumColumns = 3;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 16;
    };

    template<>
    struct TShaderParameterTypeInfo<uvec4>
    {
        static constexpr EUniformBufferMemberType BaseType = EUniformBufferMemberType::UBMT_uvec4;
        static constexpr int32 NumRows = 1;
        static constexpr int32 NumColumns = 4;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 16;
    };

    template<>
    struct TShaderParameterTypeInfo<mat2>
    {
        static constexpr EUniformBufferMemberType BaseType = EUniformBufferMemberType::UBMT_mat2;
        static constexpr int32 NumRows = 2;
        static constexpr int32 NumColumns = 2;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 16;
    };

    template<>
    struct TShaderParameterTypeInfo<mat3>
    {
        static constexpr EUniformBufferMemberType BaseType = EUniformBufferMemberType::UBMT_mat3;
        static constexpr int32 NumRows = 3;
        static constexpr int32 NumColumns = 3;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 16;
    };

    template<>
    struct TShaderParameterTypeInfo<mat4>
    {
        static constexpr EUniformBufferMemberType BaseType = EUniformBufferMemberType::UBMT_mat4;
        static constexpr int32 NumRows = 4;
        static constexpr int32 NumColumns = 4;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 16;
    };

    template<typename T, size_t InNumElements>
    struct TShaderParameterTypeInfo<T[InNumElements]>
    {
        static constexpr EUniformBufferMemberType BaseType = TShaderParameterTypeInfo<T>::BaseType;
        static constexpr int32 NumRows = TShaderParameterTypeInfo<T>::NumRows;
        static constexpr int32 NumColumns = TShaderParameterTypeInfo<T>::NumColumns;
        static constexpr int32 NumElements = InNumElements;
        static constexpr int32 Alignment = 16;
    };
}

#define ALIGN(N) alignas(N) 

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
        Type MemberName[N];
#define END_UNIFORM_BUFFER_STRUCT() \
    };

template <typename InElementType, uint32 NumElements, uint32 Alignment = alignof(InElementType)>
class alignas(Alignment) TAlignedStaticArray
{
public:
    TAlignedStaticArray() { }

    InElementType &operator[](size_t index)
    {
        return Elements[index].Element;
    }

	friend bool operator==(const TAlignedStaticArray& A,const TAlignedStaticArray& B)
	{
		for(uint32 ElementIndex = 0;ElementIndex < NumElements;++ElementIndex)
		{
			if(!(A[ElementIndex] == B[ElementIndex]))
			{
				return false;
			}
		}
		return true;
	}

	friend bool operator!=(const TAlignedStaticArray& A,const TAlignedStaticArray& B)
	{
		for(uint32 ElementIndex = 0;ElementIndex < NumElements;++ElementIndex)
		{
			if(!(A[ElementIndex] == B[ElementIndex]))
			{
				return true;
			}
		}
		return false;
	}

	bool IsEmpty() const
	{
		return NumElements == 0;
	}

	int32 Num() const { return NumElements; }

private:
    struct alignas(Alignment) TArrayStorageElementAligned
	{
		TArrayStorageElementAligned() {}

		InElementType Element;
	};
    TArrayStorageElementAligned Elements[NumElements];
};
    



using namespace nilou;

    BEGIN_UNIFORM_BUFFER_STRUCT(FLightAttenParameters)
        SHADER_PARAMETER(vec4, AttenCurveParams)
        SHADER_PARAMETER(float, AttenCurveScale)
        SHADER_PARAMETER(int, AttenCurveType)
    END_UNIFORM_BUFFER_STRUCT()

    BEGIN_UNIFORM_BUFFER_STRUCT(FLightShaderParameters)
        // SHADER_PARAMETER_STRUCT_ARRAY(FLightAttenParameters, 2, lightDistAttenParams)
        SHADER_PARAMETER_STRUCT(FLightAttenParameters, lightDistAttenParams)
        SHADER_PARAMETER_STRUCT(FLightAttenParameters, lightAngleAttenParams)
        SHADER_PARAMETER(int, ShadowMappingStartIndex)
        SHADER_PARAMETER(int, ShadowMappingEndIndex)
        SHADER_PARAMETER(vec4, lightColor)
        SHADER_PARAMETER(vec3, lightPosition)
        SHADER_PARAMETER(vec3, lightDirection)
        SHADER_PARAMETER(int, lightType) 
        SHADER_PARAMETER(float, lightIntensity)
        SHADER_PARAMETER(bool, lightCastShadow)
    END_UNIFORM_BUFFER_STRUCT()

    BEGIN_UNIFORM_BUFFER_STRUCT(ArrayTest)
        SHADER_PARAMETER_ARRAY(float, 4, a)
        SHADER_PARAMETER_ARRAY(float, 4, b)
    END_UNIFORM_BUFFER_STRUCT()
