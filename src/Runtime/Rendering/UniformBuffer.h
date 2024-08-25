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
    struct TShaderParameterTypeInfo<vec2>
    {
        static constexpr EUniformBufferBaseType BaseType = UBMT_FLOAT32;
        static constexpr int32 NumRows = 1;
        static constexpr int32 NumColumns = 2;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 8;
	
	    // using TAlignedType = TAlignedTypedef<vec2, Alignment>;
    };

    template<>
    struct TShaderParameterTypeInfo<vec3>
    {
        static constexpr EUniformBufferBaseType BaseType = UBMT_FLOAT32;
        static constexpr int32 NumRows = 1;
        static constexpr int32 NumColumns = 3;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 16;
	
	    // using TAlignedType = TAlignedTypedef<vec3, Alignment>;
    };

    template<>
    struct TShaderParameterTypeInfo<vec4>
    {
        static constexpr EUniformBufferBaseType BaseType = UBMT_FLOAT32;
        static constexpr int32 NumRows = 1;
        static constexpr int32 NumColumns = 4;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 16;
	
	    // using TAlignedType = TAlignedTypedef<vec4, Alignment>;
    };

    template<>
    struct TShaderParameterTypeInfo<dvec2>
    {
        static constexpr EUniformBufferBaseType BaseType = UBMT_FLOAT64;
        static constexpr int32 NumRows = 1;
        static constexpr int32 NumColumns = 2;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 16;
	
	    // using TAlignedType = TAlignedTypedef<vec2, Alignment>;
    };

    template<>
    struct TShaderParameterTypeInfo<dvec3>
    {
        static constexpr EUniformBufferBaseType BaseType = UBMT_FLOAT64;
        static constexpr int32 NumRows = 1;
        static constexpr int32 NumColumns = 3;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 32;
	
	    // using TAlignedType = TAlignedTypedef<vec3, Alignment>;
    };

    template<>
    struct TShaderParameterTypeInfo<dvec4>
    {
        static constexpr EUniformBufferBaseType BaseType = UBMT_FLOAT64;
        static constexpr int32 NumRows = 1;
        static constexpr int32 NumColumns = 4;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 32;
	
	    // using TAlignedType = TAlignedTypedef<vec4, Alignment>;
    };

    template<>
    struct TShaderParameterTypeInfo<ivec2>
    {
        static constexpr EUniformBufferBaseType BaseType = UBMT_INT32;
        static constexpr int32 NumRows = 1;
        static constexpr int32 NumColumns = 2;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 8;
	
	    // using TAlignedType = TAlignedTypedef<ivec2, Alignment>;
    };

    template<>
    struct TShaderParameterTypeInfo<ivec3>
    {
        static constexpr EUniformBufferBaseType BaseType = UBMT_INT32;
        static constexpr int32 NumRows = 1;
        static constexpr int32 NumColumns = 3;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 16;
	
	    // using TAlignedType = TAlignedTypedef<ivec3, Alignment>;
    };

    template<>
    struct TShaderParameterTypeInfo<ivec4>
    {
        static constexpr EUniformBufferBaseType BaseType = UBMT_INT32;
        static constexpr int32 NumRows = 1;
        static constexpr int32 NumColumns = 4;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 16;
	
	    // using TAlignedType = TAlignedTypedef<ivec4, Alignment>;
    };

    template<>
    struct TShaderParameterTypeInfo<uvec2>
    {
        static constexpr EUniformBufferBaseType BaseType = UBMT_UINT32;
        static constexpr int32 NumRows = 1;
        static constexpr int32 NumColumns = 2;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 8;
	
	    // using TAlignedType = TAlignedTypedef<uvec2, Alignment>;
    };

    template<>
    struct TShaderParameterTypeInfo<uvec3>
    {
        static constexpr EUniformBufferBaseType BaseType = UBMT_UINT32;
        static constexpr int32 NumRows = 1;
        static constexpr int32 NumColumns = 3;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 16;
	
	    // using TAlignedType = TAlignedTypedef<uvec3, Alignment>;
    };

    template<>
    struct TShaderParameterTypeInfo<uvec4>
    {
        static constexpr EUniformBufferBaseType BaseType = UBMT_UINT32;
        static constexpr int32 NumRows = 1;
        static constexpr int32 NumColumns = 4;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 16;
	
	    // using TAlignedType = TAlignedTypedef<uvec4, Alignment>;
    };

    template<>
    struct TShaderParameterTypeInfo<mat2>
    {
        static constexpr EUniformBufferBaseType BaseType = UBMT_FLOAT32;
        static constexpr int32 NumRows = 2;
        static constexpr int32 NumColumns = 2;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 16;
	
	    // using TAlignedType = TAlignedTypedef<mat2, Alignment>;
    };

    template<>
    struct TShaderParameterTypeInfo<mat3>
    {
        static constexpr EUniformBufferBaseType BaseType = UBMT_FLOAT32;
        static constexpr int32 NumRows = 3;
        static constexpr int32 NumColumns = 3;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 16;
	
	    // using TAlignedType = TAlignedTypedef<mat3, Alignment>;
    };

    template<>
    struct TShaderParameterTypeInfo<mat4>
    {
        static constexpr EUniformBufferBaseType BaseType = UBMT_FLOAT32;
        static constexpr int32 NumRows = 4;
        static constexpr int32 NumColumns = 4;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 16;
	
	    // using TAlignedType = TAlignedTypedef<mat4, Alignment>;
    };

    template<>
    struct TShaderParameterTypeInfo<dmat2>
    {
        static constexpr EUniformBufferBaseType BaseType = UBMT_FLOAT64;
        static constexpr int32 NumRows = 2;
        static constexpr int32 NumColumns = 2;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 32;
	
	    // using TAlignedType = TAlignedTypedef<mat2, Alignment>;
    };

    template<>
    struct TShaderParameterTypeInfo<dmat3>
    {
        static constexpr EUniformBufferBaseType BaseType = UBMT_FLOAT64;
        static constexpr int32 NumRows = 3;
        static constexpr int32 NumColumns = 3;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 32;
	
	    // using TAlignedType = TAlignedTypedef<mat3, Alignment>;
    };

    template<>
    struct TShaderParameterTypeInfo<dmat4>
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
        struct NSTRUCT TypeName \
        { \
            GENERATED_STRUCT_BODY()
    #define SHADER_PARAMETER(Type, MemberName) \
            alignas(TShaderParameterTypeInfo<Type>::Alignment) NPROPERTY() Type MemberName;
    #define SHADER_PARAMETER_ARRAY(Type, N, MemberName) \
            NPROPERTY() \
            TAlignedStaticArray<Type, N, TShaderParameterTypeInfo<Type[N]>::Alignment> MemberName;
    #define SHADER_PARAMETER_STRUCT(Type, MemberName) \
            NPROPERTY() \
            Type MemberName;
    #define SHADER_PARAMETER_STRUCT_ARRAY(Type, N, MemberName) \
            NPROPERTY() \
            std::array<Type, N> MemberName;
    #define END_UNIFORM_BUFFER_STRUCT() \
        };

    class FUniformBuffer : public FRenderResource
    {
    public:
        FUniformBuffer(EUniformBufferUsage InUsage) : UniformBufferRDG(nullptr), Usage(InUsage) { }
        RDGBuffer* GetRDG() const { return UniformBufferRDG; }
        RHIBuffer* GetRHI() const { return UniformBufferRDG->Resolve(); }

    protected:

        void InitRHI_impl(RenderGraph& Graph);
        void UploadData_impl(RenderGraph& Graph, const void* Data, uint32 DataSize);

        RDGBuffer* UniformBufferRDG;
        uint32 Size;
        EUniformBufferUsage Usage;
    };

    template<class UniformBufferStruct>
    class TUniformBuffer : public FUniformBuffer
    {
    public:
        TUniformBuffer(EUniformBufferUsage InUsage)
            : FUniformBuffer(InUsage)
        { 
            Size = sizeof(UniformBufferStruct);
        }

        /** Begin FRenderResource Interface */
        virtual void InitRHI(RenderGraph& Graph) override
        {
            // FRenderResource::InitRHI(Graph);
            // UniformBufferRHI = FDynamicRHI::GetDynamicRHI()->RHICreateUniformBuffer(Size, Usage, &Data);
            FRenderResource::InitRHI(Graph);
            InitRHI_impl(Graph);
            UploadData_impl(Graph, &Data, sizeof(UniformBufferStruct));
        }
        virtual void ReleaseRHI() override
        {
            // UniformBufferRHI = nullptr;
            FRenderResource::ReleaseRHI();
        }
        /** End FRenderResource Interface */

        void UpdateUniformBuffer(RenderGraph& Graph)
        {
            UploadData_impl(Graph, &Data, sizeof(UniformBufferStruct));
        }

        void UpdateUniformBuffer()
        {
            
        }

        UniformBufferStruct Data;
    };

    template<class UniformBufferStruct>
    using TUniformBufferRef = std::shared_ptr<TUniformBuffer<UniformBufferStruct>>;

    template<class UniformBufferStruct>
    inline TUniformBufferRef<UniformBufferStruct> CreateUniformBuffer()
    {
        return TUniformBufferRef<UniformBufferStruct>(std::make_shared<TUniformBuffer<UniformBufferStruct>>());
    }

}