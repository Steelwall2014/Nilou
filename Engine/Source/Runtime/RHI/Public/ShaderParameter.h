#pragma once

#include <string>
#include <vector>

#include "HAL/Platform.h"
#include "RHIResources.h"

namespace nilou {

    // /** Alignements tools because alignas() does not work on type in clang. */
    template<typename T, int32 Alignment>
    class TAlignedTypedef;

    #define IMPLEMENT_ALIGNED_TYPE(Alignment) \
        template<typename T> \
        class TAlignedTypedef<T,Alignment> \
        { \
        public: \
            typedef MS_ALIGN(Alignment) T Type GCC_ALIGN(Alignment); \
        };

    template<typename T, int32 Alignment>
    using TAlignedType = TAlignedTypedef<T, Alignment>::Type;

    IMPLEMENT_ALIGNED_TYPE(1);
    IMPLEMENT_ALIGNED_TYPE(2);
    IMPLEMENT_ALIGNED_TYPE(4);
    IMPLEMENT_ALIGNED_TYPE(8);
    IMPLEMENT_ALIGNED_TYPE(16);
    IMPLEMENT_ALIGNED_TYPE(32);
    #undef IMPLEMENT_ALIGNED_TYPE

    enum class EShaderParameterType
    {
        SPT_None,
        SPT_Sampler,
        // SPT_Uniform,
        SPT_UniformBuffer,
        // SPT_ShaderStructureBuffer,
        SPT_Image,
        // SPT_AtomicUint,
        // SPT_Uint,
        // SPT_Int,
        // SPT_Float

    };

    class FShaderParameterInfo
    {
    public:
        std::string ParameterName;
        EShaderParameterType ParameterType;
        int16 BindingPoint;
        // uint16 Size;

        FShaderParameterInfo()
            : ParameterName("")
            , BindingPoint(-1)
            , ParameterType(EShaderParameterType::SPT_UniformBuffer)
        {
        }

        FShaderParameterInfo(const std::string &InParameterName, int16 InBindingPoint, EShaderParameterType InParameterType/*, uint16 InSize*/)
            : ParameterName(InParameterName)
            , BindingPoint(InBindingPoint)
            , ParameterType(InParameterType)
        {
        }

        bool operator<(const FShaderParameterInfo& Other) const
        {
            return ParameterName < Other.ParameterName;
        }
        
        // inline bool operator==(const FShaderParameterInfo& Rhs) const
        // {
        //     return BindingPoint == Rhs.BindingPoint
        //         && Size == Rhs.Size;
        // }

        // inline bool operator<(const FShaderParameterInfo& Rhs) const
        // {
        //     return BindingPoint < Rhs.BindingPoint;
        // }
    };

    enum class EShaderDataLayout
    {
        Std140,
        Std430,
        Opaque,
    };
    constexpr EShaderDataLayout Std140Layout = EShaderDataLayout::Std140;
    constexpr EShaderDataLayout Std430Layout = EShaderDataLayout::Std430;
    constexpr EShaderDataLayout OpaqueLayout = EShaderDataLayout::Opaque;

    enum class EUniformBufferBaseType2 : uint8
    {
        None,

        // Invalid type when trying to use bool, to have explicit error message to programmer on why
        // they shouldn't use bool in shader parameter structures.
        Bool,

        // Parameter types.
        Int32,
        UInt32,
        Float32,
        Float64,

        // Resource types.
        Texture,
        Sampler,
        TextureSampler,

        // Buffer types.
        Buffer,

        // Nested structure.
        NestedStruct,
    };

    enum class EShaderPrecisionModifier2 : uint8
    {
        Float,
        Half,
        Fixed,
        Invalid,
    };

    struct FShaderParametersMetadata2
    {
        struct FMember
        {
            // Vulkan descriptor set binding index for resource entries (0,1,2,…). Uniform-block payload
            // fields also use 0 here; tell them apart from descriptors using BaseType (numeric vs Texture/Buffer/…)
            // and Name (e.g. AutomaticallyIntroducedUniformBuffer).
            int32 BindingIndex;
            // Member name as it appears in the shader source.
            std::string Name;
            // Byte offset of this member within the owning layout struct.
            int32 Offset;
            // Underlying data or resource category (e.g. Float32, Texture, Sampler, Buffer …).
            EUniformBufferBaseType2 BaseType;
            // Floating-point precision qualifier; Invalid for non-float and resource types.
            EShaderPrecisionModifier2 Precision;
            // Number of rows; 1 for scalars, vectors, and resource types; >1 for matrix types.
            uint32 NumRows;
            // Number of columns; equals the vector width for vectors; equals the column count for matrices; 1 for scalars and resources.
            uint32 NumColumns;
            // Array element count; 1 for non-array members.
            uint32 NumElements;
            // Byte stride between array elements; 0 for non-array members.
            uint32 ArrayStride;
        };
        FShaderParametersMetadata2(const std::string& InStructTypeName,
                                   const std::string& InFileName,
                                   int32 InFileLine,
                                   RHIDescriptorSetLayoutRef InDescriptorSetLayout,
                                   const std::vector<FMember>& InMembers)
            : StructTypeName(InStructTypeName)
            , FileName(InFileName)
            , FileLine(InFileLine)
            , DescriptorSetLayout(InDescriptorSetLayout)
            , Members(InMembers)
        {
        }
        const std::string StructTypeName;
        const std::string FileName;
        int32 FileLine;

        const std::vector<FMember> Members;
        RHIDescriptorSetLayoutRef DescriptorSetLayout = nullptr;
    };

    struct RHI_API FShaderParameterRegistry
    {
        static void RegisterTypes();
        FShaderParameterRegistry(std::function<std::pair<std::string, FShaderParametersMetadata2*>()> GetStructNameAndMetadata);
    };
    #define REGISTER_SHADER_PARAMETER_STRUCT(StructName) \
        static FShaderParameterRegistry UniqueRegister_##StructName( \
            []() -> std::pair<std::string, FShaderParametersMetadata2*> \
            { \
                return std::make_pair(#StructName, GetShaderParametersMetadata<StructName>()); \
            } \
        );

    template <template <EShaderDataLayout> typename T>
    FShaderParametersMetadata2* GetShaderParametersMetadata();
    RHI_API FShaderParametersMetadata2* GetShaderParametersMetadata(const std::string& StructName);


}