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
#include "Math/Maths.h"
#include "DynamicRHI.h"
#include "ShaderParameter.h"

    
/** Alignment of the shader parameters struct is required to be 16-byte boundaries. */
#define SHADER_PARAMETER_STRUCT_ALIGNMENT 16

/** The alignment in bytes between elements of array shader parameters. */
#define SHADER_PARAMETER_ARRAY_ELEMENT_ALIGNMENT 16

namespace nilou {

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

        UBMT_TEXTURE,
        UBMT_SAMPLER,
        UBMT_TEXTURE_SAMPLER,

        // Nested structure.
        UBMT_NESTED_STRUCT,

        EUniformBufferBaseType_Num,
    };

    class FShaderParametersMetadata;

    /** Template to transcode some meta data information for a type <TypeParameter> not specific to shader parameters API. */
    template<typename TypeParameter>
    struct TShaderParameterTypeInfo
    {
        /** Defines what the type actually is. */
        static constexpr EUniformBufferBaseType BaseType = UBMT_INVALID;

        /** Defines the number rows and columns for vector or matrix based data typed. */
        static constexpr int32 NumRows = 1;
        static constexpr int32 NumColumns = 1;
    
        /** Defines the number of elements in an array fashion. 0 means this is not a TStaticArray,
         * which therefor means there is 1 element.
         */
        static constexpr int32 NumElements = 0;
    
        /** Defines the alignment of the elements in bytes. */
        static constexpr int32 Alignment = alignof(TypeParameter);
    
        /** Defines whether this element is stored in constant buffer or not.
         * This informations is usefull to ensure at compile time everything in the
         * structure get defined at the end of the structure, to reduce as much as possible
         * the size of the constant buffer.
         */
        static constexpr bool bIsStoredInConstantBuffer = true;

        /** Type that is actually alligned. */
        using TAlignedType = TypeParameter;

        static const FShaderParametersMetadata* GetStructMetadata() { return TypeParameter::FTypeInfo::GetStructMetadata(); }

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
        static constexpr bool bIsStoredInConstantBuffer = true;
	
	    using TAlignedType = TAlignedTypedef<uint32, Alignment>::Type;

        static const FShaderParametersMetadata* GetStructMetadata() { return nullptr; }
    };

    template<>
    struct TShaderParameterTypeInfo<int32>
    {
        static constexpr EUniformBufferBaseType BaseType = UBMT_INT32;
        static constexpr int32 NumRows = 1;
        static constexpr int32 NumColumns = 1;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 4;
        static constexpr bool bIsStoredInConstantBuffer = true;
	
	    using TAlignedType = TAlignedTypedef<int32, Alignment>::Type;

        static const FShaderParametersMetadata* GetStructMetadata() { return nullptr; }
    };

    template<>
    struct TShaderParameterTypeInfo<float>
    {
        static constexpr EUniformBufferBaseType BaseType = UBMT_FLOAT32;
        static constexpr int32 NumRows = 1;
        static constexpr int32 NumColumns = 1;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 4;
        static constexpr bool bIsStoredInConstantBuffer = true;
	
	    using TAlignedType = TAlignedTypedef<float, Alignment>::Type;

        static const FShaderParametersMetadata* GetStructMetadata() { return nullptr; }
    };

    template<>
    struct TShaderParameterTypeInfo<FVector2f>
    {
        static constexpr EUniformBufferBaseType BaseType = UBMT_FLOAT32;
        static constexpr int32 NumRows = 1;
        static constexpr int32 NumColumns = 2;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 8;
        static constexpr bool bIsStoredInConstantBuffer = true;
	
	    using TAlignedType = TAlignedTypedef<FVector2f, Alignment>::Type;

        static const FShaderParametersMetadata* GetStructMetadata() { return nullptr; }
    };

    template<>
    struct TShaderParameterTypeInfo<FVector3f>
    {
        static constexpr EUniformBufferBaseType BaseType = UBMT_FLOAT32;
        static constexpr int32 NumRows = 1;
        static constexpr int32 NumColumns = 3;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 16;
        static constexpr bool bIsStoredInConstantBuffer = true;
	
	    using TAlignedType = TAlignedTypedef<FVector3f, Alignment>::Type;

        static const FShaderParametersMetadata* GetStructMetadata() { return nullptr; }
    };

    template<>
    struct TShaderParameterTypeInfo<FVector4f>
    {
        static constexpr EUniformBufferBaseType BaseType = UBMT_FLOAT32;
        static constexpr int32 NumRows = 1;
        static constexpr int32 NumColumns = 4;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 16;
        static constexpr bool bIsStoredInConstantBuffer = true;
	
	    using TAlignedType = TAlignedTypedef<FVector4f, Alignment>::Type;

        static const FShaderParametersMetadata* GetStructMetadata() { return nullptr; }
    };

    template<>
    struct TShaderParameterTypeInfo<FVector2>
    {
        static constexpr EUniformBufferBaseType BaseType = UBMT_FLOAT64;
        static constexpr int32 NumRows = 1;
        static constexpr int32 NumColumns = 2;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 16;
        static constexpr bool bIsStoredInConstantBuffer = true;
	
	    using TAlignedType = TAlignedTypedef<FVector2, Alignment>::Type;

        static const FShaderParametersMetadata* GetStructMetadata() { return nullptr; }
    };

    template<>
    struct TShaderParameterTypeInfo<FVector>
    {
        static constexpr EUniformBufferBaseType BaseType = UBMT_FLOAT64;
        static constexpr int32 NumRows = 1;
        static constexpr int32 NumColumns = 3;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 32;
        static constexpr bool bIsStoredInConstantBuffer = true;
	
	    using TAlignedType = TAlignedTypedef<FVector, Alignment>::Type;

        static const FShaderParametersMetadata* GetStructMetadata() { return nullptr; }
    };

    template<>
    struct TShaderParameterTypeInfo<FVector4>
    {
        static constexpr EUniformBufferBaseType BaseType = UBMT_FLOAT64;
        static constexpr int32 NumRows = 1;
        static constexpr int32 NumColumns = 4;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 32;
        static constexpr bool bIsStoredInConstantBuffer = true;
	
	    using TAlignedType = TAlignedTypedef<FVector4, Alignment>::Type;

        static const FShaderParametersMetadata* GetStructMetadata() { return nullptr; }
    };

    template<>
    struct TShaderParameterTypeInfo<FIntVector2>
    {
        static constexpr EUniformBufferBaseType BaseType = UBMT_INT32;
        static constexpr int32 NumRows = 1;
        static constexpr int32 NumColumns = 2;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 8;
        static constexpr bool bIsStoredInConstantBuffer = true;
	
	    using TAlignedType = TAlignedTypedef<FIntVector2, Alignment>::Type;

        static const FShaderParametersMetadata* GetStructMetadata() { return nullptr; }
    };

    template<>
    struct TShaderParameterTypeInfo<FIntVector>
    {
        static constexpr EUniformBufferBaseType BaseType = UBMT_INT32;
        static constexpr int32 NumRows = 1;
        static constexpr int32 NumColumns = 3;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 16;
        static constexpr bool bIsStoredInConstantBuffer = true;
	
	    using TAlignedType = TAlignedTypedef<FIntVector, Alignment>::Type;

        static const FShaderParametersMetadata* GetStructMetadata() { return nullptr; }
    };

    template<>
    struct TShaderParameterTypeInfo<FIntVector4>
    {
        static constexpr EUniformBufferBaseType BaseType = UBMT_INT32;
        static constexpr int32 NumRows = 1;
        static constexpr int32 NumColumns = 4;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 16;
        static constexpr bool bIsStoredInConstantBuffer = true;
	
	    using TAlignedType = TAlignedTypedef<FIntVector4, Alignment>::Type;

        static const FShaderParametersMetadata* GetStructMetadata() { return nullptr; }
    };

    template<>
    struct TShaderParameterTypeInfo<FUIntVector2>
    {
        static constexpr EUniformBufferBaseType BaseType = UBMT_UINT32;
        static constexpr int32 NumRows = 1;
        static constexpr int32 NumColumns = 2;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 8;
        static constexpr bool bIsStoredInConstantBuffer = true;

	    using TAlignedType = TAlignedTypedef<FUIntVector2, Alignment>::Type;

        static const FShaderParametersMetadata* GetStructMetadata() { return nullptr; }
    };

    template<>
    struct TShaderParameterTypeInfo<FUIntVector>
    {
        static constexpr EUniformBufferBaseType BaseType = UBMT_UINT32;
        static constexpr int32 NumRows = 1;
        static constexpr int32 NumColumns = 3;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 16;
        static constexpr bool bIsStoredInConstantBuffer = true;
        
	    using TAlignedType = TAlignedTypedef<FUIntVector, Alignment>::Type;

        static const FShaderParametersMetadata* GetStructMetadata() { return nullptr; }
    };

    template<>
    struct TShaderParameterTypeInfo<FUIntVector4>
    {
        static constexpr EUniformBufferBaseType BaseType = UBMT_UINT32;
        static constexpr int32 NumRows = 1;
        static constexpr int32 NumColumns = 4;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 16;
        static constexpr bool bIsStoredInConstantBuffer = true;
        
	    using TAlignedType = TAlignedTypedef<FUIntVector4, Alignment>::Type;

        static const FShaderParametersMetadata* GetStructMetadata() { return nullptr; }
    };

    template<>
    struct TShaderParameterTypeInfo<FMatrix22f>
    {
        static constexpr EUniformBufferBaseType BaseType = UBMT_FLOAT32;
        static constexpr int32 NumRows = 2;
        static constexpr int32 NumColumns = 2;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 16;
        static constexpr bool bIsStoredInConstantBuffer = true;
        
	    using TAlignedType = TAlignedTypedef<FMatrix22f, Alignment>::Type;

        static const FShaderParametersMetadata* GetStructMetadata() { return nullptr; }
    };

    template<>
    struct TShaderParameterTypeInfo<FMatrix33f>
    {
        static constexpr EUniformBufferBaseType BaseType = UBMT_FLOAT32;
        static constexpr int32 NumRows = 3;
        static constexpr int32 NumColumns = 3;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 16;
        static constexpr bool bIsStoredInConstantBuffer = true;
	
	    using TAlignedType = TAlignedTypedef<FMatrix33f, Alignment>::Type;

        static const FShaderParametersMetadata* GetStructMetadata() { return nullptr; }
    };

    template<>
    struct TShaderParameterTypeInfo<FMatrix44f>
    {
        static constexpr EUniformBufferBaseType BaseType = UBMT_FLOAT32;
        static constexpr int32 NumRows = 4;
        static constexpr int32 NumColumns = 4;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 16;
        static constexpr bool bIsStoredInConstantBuffer = true;
	
	    using TAlignedType = TAlignedTypedef<FMatrix44f, Alignment>::Type;

        static const FShaderParametersMetadata* GetStructMetadata() { return nullptr; }
    };

    template<>
    struct TShaderParameterTypeInfo<FMatrix22>
    {
        static constexpr EUniformBufferBaseType BaseType = UBMT_FLOAT64;
        static constexpr int32 NumRows = 2;
        static constexpr int32 NumColumns = 2;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 32;
        static constexpr bool bIsStoredInConstantBuffer = true;
	
	    using TAlignedType = TAlignedTypedef<FMatrix22, Alignment>::Type;

        static const FShaderParametersMetadata* GetStructMetadata() { return nullptr; }
    };

    template<>
    struct TShaderParameterTypeInfo<FMatrix33>
    {
        static constexpr EUniformBufferBaseType BaseType = UBMT_FLOAT64;
        static constexpr int32 NumRows = 3;
        static constexpr int32 NumColumns = 3;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 32;
        static constexpr bool bIsStoredInConstantBuffer = true;
	
	    using TAlignedType = TAlignedTypedef<FMatrix33, Alignment>::Type;

        static const FShaderParametersMetadata* GetStructMetadata() { return nullptr; }
    };

    template<>
    struct TShaderParameterTypeInfo<FMatrix>
    {
        static constexpr EUniformBufferBaseType BaseType = UBMT_FLOAT64;
        static constexpr int32 NumRows = 4;
        static constexpr int32 NumColumns = 4;
        static constexpr int32 NumElements = 0;
        static constexpr int32 Alignment = 32;
        static constexpr bool bIsStoredInConstantBuffer = true;
	
	    using TAlignedType = TAlignedTypedef<FMatrix, Alignment>::Type;

        static const FShaderParametersMetadata* GetStructMetadata() { return nullptr; }
    };

    template<typename T, size_t InNumElements>
    struct TShaderParameterTypeInfo<T[InNumElements]>
    {
        static constexpr EUniformBufferBaseType BaseType = TShaderParameterTypeInfo<T>::BaseType;
        static constexpr int32 NumRows = TShaderParameterTypeInfo<T>::NumRows;
        static constexpr int32 NumColumns = TShaderParameterTypeInfo<T>::NumColumns;
        static constexpr int32 NumElements = InNumElements;
        static constexpr int32 Alignment = SHADER_PARAMETER_ARRAY_ELEMENT_ALIGNMENT;
        static constexpr bool bIsStoredInConstantBuffer = true;
	
	    using TAlignedType = TStaticArray<T, InNumElements, Alignment>;

        static const FShaderParametersMetadata* GetStructMetadata() { return TShaderParameterTypeInfo<T>::GetStructMetadata(); }
    };
    
    enum class EShaderPrecisionModifier : uint8
    {
		Float,
		Half,
		Fixed,
		Invalid
    };

    class FShaderParametersMetadata
    {
    public:
        struct FMember
        {
            FMember(
                const std::string& InName,
                const std::string& InShaderType,
                int32 InFileLine,
                uint32 InOffset,
                EUniformBufferBaseType InBaseType,
                EShaderPrecisionModifier InPrecision,
                uint32 InNumRows,
                uint32 InNumColumns,
                uint32 InNumElements,
                const FShaderParametersMetadata* InStruct)
            : Name(InName)
            , ShaderType(InShaderType)
            , FileLine(InFileLine)
            , Offset(InOffset)
            , BaseType(InBaseType)
            , Precision(InPrecision)
            , NumRows(InNumRows)
            , NumColumns(InNumColumns)
            , NumElements(InNumElements)
            , Struct(InStruct)
            {
                
            }
            std::string Name;
            std::string ShaderType;
            int32 FileLine;
            uint32 Offset;
            EUniformBufferBaseType BaseType;
            EShaderPrecisionModifier Precision;
            uint32 NumRows;
            uint32 NumColumns;
            uint32 NumElements;
            const FShaderParametersMetadata* Struct;
        };

        FShaderParametersMetadata(
            const std::string& InStructTypeName, 
            const std::string& InFileName,
            int32 InFileLine,
            uint32 InSize, 
            const TArray<FMember>& InMembers)
        : StructTypeName(InStructTypeName)
        , FileName(InFileName)
        , FileLine(InFileLine)
        , Size(InSize)
        , Members(InMembers)
        {

        }

        const std::string StructTypeName; 
        const std::string& FileName;
        int32 FileLine;
        uint32 Size;
        const TArray<FMember> Members;
    };

    extern TArray<std::unique_ptr<FShaderParametersMetadata>> ShaderParametersMetadataRegistration;

    #define BEGIN_UNIFORM_BUFFER_STRUCT(TypeName) \
        BEGIN_SHADER_PARAMETER_STRUCT(TypeName, )
    #define END_UNIFORM_BUFFER_STRUCT() \
        END_SHADER_PARAMETER_STRUCT()

    #define BEGIN_SHADER_PARAMETER_STRUCT(StructTypeName, DllStorage) \
        MS_ALIGN(SHADER_PARAMETER_STRUCT_ALIGNMENT) class StructTypeName \
        { \
        public: \
            DllStorage StructTypeName () { } \
            struct FTypeInfo { \
                static constexpr int32 NumRows = 1; \
                static constexpr int32 NumColumns = 1; \
                static constexpr int32 NumElements = 0; \
                static constexpr int32 Alignment = SHADER_PARAMETER_STRUCT_ALIGNMENT; \
                static constexpr bool bIsStoredInConstantBuffer = true; \
                static constexpr const char* const FileName = __builtin_FILE(); \
                static constexpr int32 FileLine = __builtin_LINE(); \
                using TAlignedType = StructTypeName; \
                static const FShaderParametersMetadata* GetStructMetadata() \
                { \
                    static FShaderParametersMetadata StaticStructMetadata(\
                        #StructTypeName, \
                        FTypeInfo::FileName, \
                        FTypeInfo::FileLine, \
                        sizeof(StructTypeName), \
                        StructTypeName::zzGetMembers()); \
                    return &StaticStructMetadata; \
                } \
            }; \
        private: \
            typedef StructTypeName zzTThisStruct; \
            struct zzFirstMemberId { enum { HasDeclaredResource = 0 }; }; \
            typedef void* zzFuncPtr; \
            typedef zzFuncPtr(*zzMemberFunc)(zzFirstMemberId, TArray<FShaderParametersMetadata::FMember>*); \
            static zzFuncPtr zzAppendMemberGetPrev(zzFirstMemberId, TArray<FShaderParametersMetadata::FMember>*) \
            { \
                return nullptr; \
            } \
            typedef zzFirstMemberId

    #define SHADER_PARAMETER(MemberType, MemberName) \
        INTERNAL_SHADER_PARAMETER_EXPLICIT(TShaderParameterTypeInfo<MemberType>::BaseType, TShaderParameterTypeInfo<MemberType>, MemberType, MemberName, , EShaderPrecisionModifier::Float, #MemberType)

    #define SHADER_PARAMETER_ARRAY(MemberType, MemberName, ArrayDecl) \
        INTERNAL_SHADER_PARAMETER_EXPLICIT(TShaderParameterTypeInfo<MemberType ArrayDecl>::BaseType, TShaderParameterTypeInfo<MemberType ArrayDecl>, MemberType, MemberName, , EShaderPrecisionModifier::Float, #MemberType)

    #define SHADER_PARAMETER_STRUCT(MemberType, MemberName) \
        INTERNAL_SHADER_PARAMETER_EXPLICIT(UBMT_NESTED_STRUCT, TShaderParameterTypeInfo<MemberType>, MemberType, MemberName, , EShaderPrecisionModifier::Float, #MemberType)

    #define SHADER_PARAMETER_STRUCT_ARRAY(MemberType, MemberName, ArrayDecl) \
        INTERNAL_SHADER_PARAMETER_EXPLICIT(UBMT_NESTED_STRUCT, TShaderParameterTypeInfo<MemberType ArrayDecl>, MemberType, MemberName, , EShaderPrecisionModifier::Float, #MemberType)

    #define SHADER_PARAMETER_RDG_TEXTURE(MemberType, MemberName) \
        INTERNAL_SHADER_PARAMETER_EXPLICIT(UBMT_TEXTURE, TShaderParameterTypeInfo<RDGTextureView*>, RDGTextureView*, MemberName, = nullptr, EShaderPrecisionModifier::Float, #MemberType)

    #define INTERNAL_SHADER_PARAMETER_EXPLICIT(BaseType, TypeInfo, MemberType, MemberName, DefaultValue, Precision, OptionalShaderType) \
        zzMemberId##MemberName; \
        public: \
            alignas(TypeInfo::Alignment) TypeInfo::TAlignedType MemberName DefaultValue; \
            static_assert(BaseType != UBMT_INVALID, "Invalid type " #MemberType " of member " #MemberName "."); \
        private: \
            struct zzNextMemberId##MemberName { enum { HasDeclaredResource = zzMemberId##MemberName::HasDeclaredResource || !TypeInfo::bIsStoredInConstantBuffer }; }; \
            static zzFuncPtr zzAppendMemberGetPrev(zzNextMemberId##MemberName, TArray<FShaderParametersMetadata::FMember>* Members) \
            { \
                Members->Add(FShaderParametersMetadata::FMember( \
                    #MemberName, \
                    OptionalShaderType, \
                    __LINE__, \
                    offsetof(zzTThisStruct,MemberName), \
                    EUniformBufferBaseType(BaseType), \
                    Precision, \
                    TypeInfo::NumRows, \
                    TypeInfo::NumColumns, \
                    TypeInfo::NumElements, \
                    TypeInfo::GetStructMetadata() \
                    )); \
                zzFuncPtr(*PrevFunc)(zzMemberId##MemberName, TArray<FShaderParametersMetadata::FMember>*); \
                PrevFunc = zzAppendMemberGetPrev; \
                return (zzFuncPtr)PrevFunc; \
            } \
            typedef zzNextMemberId##MemberName

    #define END_SHADER_PARAMETER_STRUCT() \
            zzLastMemberId; \
        public: \
            static TArray<FShaderParametersMetadata::FMember> zzGetMembers() { \
                TArray<FShaderParametersMetadata::FMember> Members; \
                zzFuncPtr(*LastFunc)(zzLastMemberId, TArray<FShaderParametersMetadata::FMember>*); \
                LastFunc = zzAppendMemberGetPrev; \
                zzFuncPtr Ptr = (zzFuncPtr)LastFunc; \
                do \
                { \
                    Ptr = reinterpret_cast<zzMemberFunc>(Ptr)(zzFirstMemberId(), &Members); \
                } while (Ptr); \
                std::reverse(Members.begin(), Members.end()); \
                return Members; \
            } \
        } GCC_ALIGN(SHADER_PARAMETER_STRUCT_ALIGNMENT);
}