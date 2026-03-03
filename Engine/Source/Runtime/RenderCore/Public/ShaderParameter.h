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

    struct FShaderParametersMetadata2
    {
        enum class EOpaqueResourceType
        {
            None,
            TextureView,
            SamplerState,
            CombinedTextureSampler,
            Buffer,
        };
        struct FOpaqueResource
        {
            int32 BindingIndex = -1;
            int32 Offset = -1;  // Offset in cpp struct
            EOpaqueResourceType ResourceType = EOpaqueResourceType::None;
        };
        FShaderParametersMetadata2(
            const std::string& InStructTypeName, 
            const std::string& InFileName,
            int32 InFileLine,
            RHIDescriptorSetLayoutRef InDescriptorSetLayout,
            const std::vector<FOpaqueResource>& InOpaqueResources)
        : StructTypeName(InStructTypeName)
        , FileName(InFileName)
        , FileLine(InFileLine)
        , DescriptorSetLayout(InDescriptorSetLayout)
        , OpaqueResources(InOpaqueResources)
        {
    
        }
        const std::string StructTypeName; 
        const std::string FileName;
        int32 FileLine;
        
        std::vector<FOpaqueResource> OpaqueResources;
        RHIDescriptorSetLayoutRef DescriptorSetLayout = nullptr;
    };

    struct RENDERCORE_API FShaderParameterRegistry
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
    RENDERCORE_API FShaderParametersMetadata2* GetShaderParametersMetadata(const std::string& StructName);

    template <template <EShaderDataLayout> typename T>
    struct TParameterBlock : public T<Std140Layout>, public T<OpaqueLayout>
    {
        T<Std140Layout>& GetNonOpaqueFields()
        {
            return *static_cast<T<Std140Layout>*>(this);
        }
        T<EShaderDataLayout::Opaque>& GetOpaqueFields()
        {
            return *static_cast<T<OpaqueLayout>*>(this);
        }

        const T<Std140Layout>& GetNonOpaqueFields() const
        {
            return *static_cast<const T<Std140Layout>*>(this);
        }
        const T<EShaderDataLayout::Opaque>& GetOpaqueFields() const
        {
            return *static_cast<const T<OpaqueLayout>*>(this);
        }

        class RDGDescriptorSet* DescriptorSet = nullptr;
    };


}