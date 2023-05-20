#pragma once
#include <memory>
#include <string>
#include <map>
#include <string_view>
#include <type_traits>
// #include <vcruntime.h>
#include <vector>

#include "Common/CoreUObject/Object.h"
#include "Common/AssertionMacros.h"
#include "Common/Containers/Array.h"
#include "Platform.h"
#include "RHIDefinitions.h"
#include "RenderResource.h"
#include "Common/Maths.h"
#include "DynamicRHI.h"

    
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

    enum class EUniformBufferMemberType
    {
        UBMT_Null,
        // UBMT_bool,
        UBMT_int,
        UBMT_uint,
        UBMT_float,
        UBMT_vec2,
        UBMT_vec3,
        UBMT_vec4,
        UBMT_dvec2,
        UBMT_dvec3,
        UBMT_dvec4,
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
        UBMT_dmat2,
        UBMT_dmat3,
        UBMT_dmat4,
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

	    // using TAlignedType = TypeParameter;
        static_assert(std::is_same<TypeParameter, bool>::value != true, "Boolean type for uniform buffer is not supported, you have to use int32 in cpp and bool in glsl");
    };

    // template<>
    // struct TShaderParameterTypeInfo<bool>
    // {
    //     static constexpr EUniformBufferMemberType BaseType = EUniformBufferMemberType::UBMT_bool;
    //     static constexpr int32 NumRows = 1;
    //     static constexpr int32 NumColumns = 1;
    //     static constexpr int32 NumElements = 0;
    //     static constexpr int32 Alignment = 4;
	
	//     // using TAlignedType = TAlignedTypedef<int32, Alignment>;
    // };

    template<>
    struct TShaderParameterTypeInfo<uint32>
    {
        static constexpr EUniformBufferMemberType BaseType = EUniformBufferMemberType::UBMT_uint;
        static constexpr int32 NumRows = 1;
        static constexpr int32 NumColumns = 1;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 4;
	
	    // using TAlignedType = TAlignedTypedef<uint32, Alignment>;
    };

    template<>
    struct TShaderParameterTypeInfo<int32>
    {
        static constexpr EUniformBufferMemberType BaseType = EUniformBufferMemberType::UBMT_int;
        static constexpr int32 NumRows = 1;
        static constexpr int32 NumColumns = 1;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 4;
	
	    // using TAlignedType = TAlignedTypedef<int32, Alignment>;
    };

    template<>
    struct TShaderParameterTypeInfo<float>
    {
        static constexpr EUniformBufferMemberType BaseType = EUniformBufferMemberType::UBMT_float;
        static constexpr int32 NumRows = 1;
        static constexpr int32 NumColumns = 1;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 4;
	
	    // using TAlignedType = TAlignedTypedef<float, Alignment>;
    };

    template<>
    struct TShaderParameterTypeInfo<vec2>
    {
        static constexpr EUniformBufferMemberType BaseType = EUniformBufferMemberType::UBMT_vec2;
        static constexpr int32 NumRows = 1;
        static constexpr int32 NumColumns = 2;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 8;
	
	    // using TAlignedType = TAlignedTypedef<vec2, Alignment>;
    };

    template<>
    struct TShaderParameterTypeInfo<vec3>
    {
        static constexpr EUniformBufferMemberType BaseType = EUniformBufferMemberType::UBMT_vec3;
        static constexpr int32 NumRows = 1;
        static constexpr int32 NumColumns = 3;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 16;
	
	    // using TAlignedType = TAlignedTypedef<vec3, Alignment>;
    };

    template<>
    struct TShaderParameterTypeInfo<vec4>
    {
        static constexpr EUniformBufferMemberType BaseType = EUniformBufferMemberType::UBMT_vec4;
        static constexpr int32 NumRows = 1;
        static constexpr int32 NumColumns = 4;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 16;
	
	    // using TAlignedType = TAlignedTypedef<vec4, Alignment>;
    };

    template<>
    struct TShaderParameterTypeInfo<dvec2>
    {
        static constexpr EUniformBufferMemberType BaseType = EUniformBufferMemberType::UBMT_dvec2;
        static constexpr int32 NumRows = 1;
        static constexpr int32 NumColumns = 2;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 16;
	
	    // using TAlignedType = TAlignedTypedef<vec2, Alignment>;
    };

    template<>
    struct TShaderParameterTypeInfo<dvec3>
    {
        static constexpr EUniformBufferMemberType BaseType = EUniformBufferMemberType::UBMT_dvec3;
        static constexpr int32 NumRows = 1;
        static constexpr int32 NumColumns = 3;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 32;
	
	    // using TAlignedType = TAlignedTypedef<vec3, Alignment>;
    };

    template<>
    struct TShaderParameterTypeInfo<dvec4>
    {
        static constexpr EUniformBufferMemberType BaseType = EUniformBufferMemberType::UBMT_dvec4;
        static constexpr int32 NumRows = 1;
        static constexpr int32 NumColumns = 4;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 32;
	
	    // using TAlignedType = TAlignedTypedef<vec4, Alignment>;
    };

    template<>
    struct TShaderParameterTypeInfo<bvec2>
    {
        static constexpr EUniformBufferMemberType BaseType = EUniformBufferMemberType::UBMT_bvec2;
        static constexpr int32 NumRows = 1;
        static constexpr int32 NumColumns = 2;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 8;
	
	    // using TAlignedType = TAlignedTypedef<bvec2, Alignment>;
    };

    template<>
    struct TShaderParameterTypeInfo<bvec3>
    {
        static constexpr EUniformBufferMemberType BaseType = EUniformBufferMemberType::UBMT_bvec3;
        static constexpr int32 NumRows = 1;
        static constexpr int32 NumColumns = 3;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 16;
	
	    // using TAlignedType = TAlignedTypedef<bvec3, Alignment>;
    };

    template<>
    struct TShaderParameterTypeInfo<bvec4>
    {
        static constexpr EUniformBufferMemberType BaseType = EUniformBufferMemberType::UBMT_bvec4;
        static constexpr int32 NumRows = 1;
        static constexpr int32 NumColumns = 4;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 16;
	
	    // using TAlignedType = TAlignedTypedef<bvec4, Alignment>;
    };

    template<>
    struct TShaderParameterTypeInfo<ivec2>
    {
        static constexpr EUniformBufferMemberType BaseType = EUniformBufferMemberType::UBMT_ivec2;
        static constexpr int32 NumRows = 1;
        static constexpr int32 NumColumns = 2;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 8;
	
	    // using TAlignedType = TAlignedTypedef<ivec2, Alignment>;
    };

    template<>
    struct TShaderParameterTypeInfo<ivec3>
    {
        static constexpr EUniformBufferMemberType BaseType = EUniformBufferMemberType::UBMT_ivec3;
        static constexpr int32 NumRows = 1;
        static constexpr int32 NumColumns = 3;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 16;
	
	    // using TAlignedType = TAlignedTypedef<ivec3, Alignment>;
    };

    template<>
    struct TShaderParameterTypeInfo<ivec4>
    {
        static constexpr EUniformBufferMemberType BaseType = EUniformBufferMemberType::UBMT_ivec4;
        static constexpr int32 NumRows = 1;
        static constexpr int32 NumColumns = 4;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 16;
	
	    // using TAlignedType = TAlignedTypedef<ivec4, Alignment>;
    };

    template<>
    struct TShaderParameterTypeInfo<uvec2>
    {
        static constexpr EUniformBufferMemberType BaseType = EUniformBufferMemberType::UBMT_uvec2;
        static constexpr int32 NumRows = 1;
        static constexpr int32 NumColumns = 2;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 8;
	
	    // using TAlignedType = TAlignedTypedef<uvec2, Alignment>;
    };

    template<>
    struct TShaderParameterTypeInfo<uvec3>
    {
        static constexpr EUniformBufferMemberType BaseType = EUniformBufferMemberType::UBMT_uvec3;
        static constexpr int32 NumRows = 1;
        static constexpr int32 NumColumns = 3;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 16;
	
	    // using TAlignedType = TAlignedTypedef<uvec3, Alignment>;
    };

    template<>
    struct TShaderParameterTypeInfo<uvec4>
    {
        static constexpr EUniformBufferMemberType BaseType = EUniformBufferMemberType::UBMT_uvec4;
        static constexpr int32 NumRows = 1;
        static constexpr int32 NumColumns = 4;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 16;
	
	    // using TAlignedType = TAlignedTypedef<uvec4, Alignment>;
    };

    template<>
    struct TShaderParameterTypeInfo<mat2>
    {
        static constexpr EUniformBufferMemberType BaseType = EUniformBufferMemberType::UBMT_mat2;
        static constexpr int32 NumRows = 2;
        static constexpr int32 NumColumns = 2;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 16;
	
	    // using TAlignedType = TAlignedTypedef<mat2, Alignment>;
    };

    template<>
    struct TShaderParameterTypeInfo<mat3>
    {
        static constexpr EUniformBufferMemberType BaseType = EUniformBufferMemberType::UBMT_mat3;
        static constexpr int32 NumRows = 3;
        static constexpr int32 NumColumns = 3;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 16;
	
	    // using TAlignedType = TAlignedTypedef<mat3, Alignment>;
    };

    template<>
    struct TShaderParameterTypeInfo<mat4>
    {
        static constexpr EUniformBufferMemberType BaseType = EUniformBufferMemberType::UBMT_mat4;
        static constexpr int32 NumRows = 4;
        static constexpr int32 NumColumns = 4;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 16;
	
	    // using TAlignedType = TAlignedTypedef<mat4, Alignment>;
    };

    template<>
    struct TShaderParameterTypeInfo<dmat2>
    {
        static constexpr EUniformBufferMemberType BaseType = EUniformBufferMemberType::UBMT_dmat2;
        static constexpr int32 NumRows = 2;
        static constexpr int32 NumColumns = 2;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 32;
	
	    // using TAlignedType = TAlignedTypedef<mat2, Alignment>;
    };

    template<>
    struct TShaderParameterTypeInfo<dmat3>
    {
        static constexpr EUniformBufferMemberType BaseType = EUniformBufferMemberType::UBMT_dmat3;
        static constexpr int32 NumRows = 3;
        static constexpr int32 NumColumns = 3;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 32;
	
	    // using TAlignedType = TAlignedTypedef<mat3, Alignment>;
    };

    template<>
    struct TShaderParameterTypeInfo<dmat4>
    {
        static constexpr EUniformBufferMemberType BaseType = EUniformBufferMemberType::UBMT_dmat4;
        static constexpr int32 NumRows = 4;
        static constexpr int32 NumColumns = 4;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 32;
	
	    // using TAlignedType = TAlignedTypedef<mat4, Alignment>;
    };

    template<typename T, size_t InNumElements>
    struct TShaderParameterTypeInfo<T[InNumElements]>
    {
        static constexpr EUniformBufferMemberType BaseType = TShaderParameterTypeInfo<T>::BaseType;
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
            Type MemberName[N];
    #define END_UNIFORM_BUFFER_STRUCT() \
        };

    class FUniformBuffer : public FRenderResource
    {
    public:
        FUniformBuffer() : UniformBufferRHI(nullptr) { }
        inline RHIUniformBuffer *GetRHI() const { return UniformBufferRHI.get();}
    protected:
        RHIUniformBufferRef UniformBufferRHI;
    };

    template<class UniformBufferStruct>
    class TUniformBuffer : public FUniformBuffer
    {
    public:
        TUniformBuffer()
            : Size(sizeof(UniformBufferStruct))
            , Usage(EUniformBufferUsage::UniformBuffer_MultiFrame)
        { }

        /** Begin FRenderResource Interface */
        virtual void InitRHI() override
        {
            FRenderResource::InitRHI();
            UniformBufferRHI = FDynamicRHI::GetDynamicRHI()->RHICreateUniformBuffer(Size, Usage, &Data);
        }
        virtual void ReleaseRHI() override
        {
            UniformBufferRHI = nullptr;
            FRenderResource::ReleaseRHI();
        }
        /** End FRenderResource Interface */

        void UpdateUniformBuffer()
        {
            FDynamicRHI::GetDynamicRHI()->RHIUpdateUniformBuffer(UniformBufferRHI, &Data);
        }

        inline EUniformBufferUsage GetUsage()
        {
            return Usage;
        }
        inline void SetUsage(EUniformBufferUsage InUsage)
        {
            Usage = InUsage;
        }

        UniformBufferStruct Data;
    protected:
        uint32 Size;
        EUniformBufferUsage Usage;
    };

    template<class UniformBufferStruct>
    using TUniformBufferRef = std::shared_ptr<TUniformBuffer<UniformBufferStruct>>;

    template<class UniformBufferStruct>
    inline TUniformBufferRef<UniformBufferStruct> CreateUniformBuffer()
    {
        return TUniformBufferRef<UniformBufferStruct>(std::make_shared<TUniformBuffer<UniformBufferStruct>>());
    }
}