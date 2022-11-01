#pragma once
#include <memory>
#include <string>
#include <map>
#include <string_view>
#include <type_traits>
#include <vcruntime.h>
#include <vector>

#include "Common/AssertionMacros.h"
#include "Platform.h"
#include "RHIDefinitions.h"
#include "RenderResource.h"
#include "Common/Maths.h"

#define BEGIN_UNIFORM_BUFFER_STRUCT(TypeName) \
    struct TypeName \
    { \
    private: \
        static TypeName DummyObject; \
        static FUniformBufferStructDeclaration *StaticDeclaration; \
        inline void SetStaticDeclaration(FUniformBufferStructDeclaration *StaticDeclaration) \
        { \
            TypeName::StaticDeclaration = StaticDeclaration; \
        } \
    public: \
        static inline const FUniformBufferStructDeclaration *GetStaticDeclaration() \
        { \
            return TypeName::StaticDeclaration; \
        } \
        inline TypeName() \
        { \
            nilou::FUniformBufferBuilder::BeginUniformBufferStruct(#TypeName);
#define SHADER_PARAMETER(Type, FieldName) \
            nilou::FUniformBufferBuilder::DeclareUniformBufferStructMember(GetDummyObject<Type>(), #FieldName);
#define SHADER_PARAMETER_ARRAY(Type, N, FieldName) \
            nilou::FUniformBufferBuilder::DeclareUniformBufferStructMember(GetDummyObject<Type[N]>(), #FieldName);
#define SHADER_PARAMETER_NESTED_STRUCT(Type, FieldName) \
            nilou::FUniformBufferBuilder::DeclareUniformBufferStructMember(#Type, #FieldName);
#define SHADER_PARAMETER_NESTED_STRUCT_ARRAY(Type, N, FieldName) \
            nilou::FUniformBufferBuilder::DeclareUniformBufferStructMember(#Type, N, #FieldName);
#define END_UNIFORM_BUFFER_STRUCT() \
            SetStaticDeclaration(&nilou::FUniformBufferBuilder::EndUniformBufferStruct());\
        } \
    }; 

#define IMPLEMENT_UNIFORM_BUFFER_STRUCT(TypeName) \
    FUniformBufferStructDeclaration *TypeName::StaticDeclaration = nullptr; \
    TypeName TypeName::DummyObject;
    
/** Alignment of the shader parameters struct is required to be 16-byte boundaries. */
#define SHADER_PARAMETER_STRUCT_ALIGNMENT 16

/** The alignment in bytes between elements of array shader parameters. */
#define SHADER_PARAMETER_ARRAY_ELEMENT_ALIGNMENT 16

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

namespace nilou {
    class FUniformBufferStructDeclaration;

    class FUniformBufferMemberDeclaration
    {
        friend class FUniformBufferBuilder;
    public:
        std::string Name;
        uint32 Offset;
        uint32 NumElements;
        EUniformBufferMemberType MemberType;
        uint32 Size;
        uint32 Padding;

        // this will be nullptr except when MemberType == UBMT_struct
        const FUniformBufferStructDeclaration *NestedStructDeclaration = nullptr;
    };

    class FUniformBufferStructDeclaration
    {
        friend class FUniformBufferBuilder;
        friend class FUniformBuffer;
    public:
        // it's actually StructSize + some padding
        inline uint32 GetBaseAlignment() const 
        { 
            return StructSize % SHADER_PARAMETER_STRUCT_ALIGNMENT == 0 ? StructSize : (StructSize / SHADER_PARAMETER_STRUCT_ALIGNMENT + 1) * SHADER_PARAMETER_STRUCT_ALIGNMENT; 
        }
        inline uint32 GetStructSize() const 
        { 
            return StructSize; 
        }
        inline const FUniformBufferMemberDeclaration &GetMemberDeclaration(const std::string &Name) const
        {
            check(HasMember(Name));
            return (*MemberDeclarations.find(Name)).second;
        }
        inline const std::map<std::string, FUniformBufferMemberDeclaration> &GetMemberDeclarations() const
        {
            return MemberDeclarations;
        }
        inline bool HasMember(const std::string &Name) const
        {
            return MemberDeclarations.find(Name) != MemberDeclarations.end();
        }
    private:
        std::map<std::string, FUniformBufferMemberDeclaration> MemberDeclarations;
        std::map<std::string, const FUniformBufferStructDeclaration *> StructMemberDeclarations;
        uint32 StructSize;
        std::string StructName;
    };

    class FUniformBufferBuilder
    {
    public:
        static void BeginUniformBufferStruct(std::string StructName);
        static void DeclareUniformBufferStructMember(const FUniformBufferStructDeclaration &StructType, std::string Name);
        static FUniformBufferStructDeclaration &EndUniformBufferStruct();

        template<typename TypeParameter, size_t NumElements>
        static void DeclareUniformBufferStructMember(TShaderParameterTypeInfo<TypeParameter[NumElements]> DummyObject, const std::string &Name)
        {
            DeclareUniformBufferStructMemberInternal(DummyObject, Name, NumElements);
        }   
        
        template<typename TypeParameter>
        static void DeclareUniformBufferStructMember(TShaderParameterTypeInfo<TypeParameter> DummyObject, const std::string &Name)
        {
            DeclareUniformBufferStructMemberInternal(DummyObject, Name, TShaderParameterTypeInfo<TypeParameter>::NumRows);
        }   

        static void DeclareUniformBufferStructMember(const std::string &NestedStructName, const std::string &Name);

        static void DeclareUniformBufferStructMember(const std::string &NestedStructName, int32 NumElements, const std::string &Name);

    private:

        template<typename TypeParameter>
        static void DeclareUniformBufferStructMemberInternal(TShaderParameterTypeInfo<TypeParameter>, const std::string &Name, int32 AlignmentMul)
        {
            // FUniformBufferBuilder *This_ = &FUniformBufferBuilder::GlobalBuilder;
            FUniformBufferMemberDeclaration MemberDeclaration;
            MemberDeclaration.MemberType = TShaderParameterTypeInfo<TypeParameter>::BaseType;
            MemberDeclaration.Name = Name;
            MemberDeclaration.NumElements = TShaderParameterTypeInfo<TypeParameter>::NumElements;
            uint32 BaseAlignment = TShaderParameterTypeInfo<TypeParameter>::Alignment;
            
            MemberDeclaration.Offset = GetCurrentOffset() % BaseAlignment == 0 ?
                                        GetCurrentOffset() :
                                        (GetCurrentOffset() / BaseAlignment + 1) * BaseAlignment;
            if (Name == "lightType")
                MemberDeclaration.Offset = 124;
            GetCurrentOffset() = MemberDeclaration.Offset + BaseAlignment * AlignmentMul;
            GetCurrentStructDeclaration().MemberDeclarations[Name] = MemberDeclaration;
        }  

        static void DeclareUniformBufferStructMemberInternal(const std::string &NestedStructName, const std::string &Name, int32 NumElements);
        
        // static FUniformBufferBuilder GlobalBuilder;
        static FUniformBufferBuilder *FUniformBufferBuilder::GetUniformBufferBuilder();
        static FUniformBufferStructDeclaration &GetCurrentStructDeclaration();
        static uint32 &GetCurrentOffset();

        FUniformBufferStructDeclaration CurrentStructDeclaration;
        uint32 CurrentOffset;
    };

    class FUniformBuffer : public FRenderResource
    {
        friend FUniformBufferBuilder;
    public:

        FUniformBuffer();
        FUniformBuffer(const FUniformBufferStructDeclaration &InDeclaration);

        ~FUniformBuffer() { delete[] Data; Data = nullptr; }

        // void Init(const FUniformBufferStructDeclaration &InDeclaration);
        // void InitWithStruct(const std::string &InStructName);

        template<class T>
        void Set(const std::string &Name, const T &Value);
        template<class T>
        void Get(const std::string &Name, T *OutValue);

        /** Begin FRenderResource Interface */
        virtual void InitRHI() override;
        /** End FRenderResource Interface */

        void UpdateUniformBuffer();

        inline RHIUniformBuffer *GetRHI() const { return UniformBufferRHI.get();}

        inline static const FUniformBufferStructDeclaration &GetStructDeclarationByName(const std::string &StructName)
        {
            check(HasStructDeclaration(StructName));
            return GetGlobalStructDeclarations()[StructName];
        }
        inline static bool HasStructDeclaration(const std::string &StructName)
        {
            return GetGlobalStructDeclarations().find(StructName) != GetGlobalStructDeclarations().end();
        }
        inline const FUniformBufferStructDeclaration &GetStructDeclaration() const
        {
            return Declaration;
        }
        inline const uint8 *GetData() const
        {
            return Data;
        }
        inline uint32 GetBufferSize() const
        {
            return Size;
        }
        inline EUniformBufferUsage GetUsage()
        {
            return Usage;
        }
        inline void SetUsage(EUniformBufferUsage InUsage)
        {
            Usage = InUsage;
        }
        uint32 ComputeOffsetAndMemberType(const std::string &Name, EUniformBufferMemberType *MemberType);

        // template<class T>
        // void Set(const std::string &Name, uint32 ArrayIndex, const T &Value);
        // template<class T>
        // void Get(const std::string &Name, uint32 ArrayIndex, T *OutValue);

    protected:

        FUniformBufferStructDeclaration Declaration;
        RHIUniformBufferRef UniformBufferRHI;
        uint8 *Data;
        uint32 Size;
        EUniformBufferUsage Usage;
        static std::map<std::string, FUniformBufferStructDeclaration> &GetGlobalStructDeclarations();
    };

    template<class UniformBufferStruct>
    class TUniformBuffer : public FUniformBuffer
    {
    public:
        TUniformBuffer() : FUniformBuffer(*UniformBufferStruct::GetStaticDeclaration()) { }
    };

    template<class UniformBufferStruct>
    class TUniformBufferRef : public std::shared_ptr<TUniformBuffer<UniformBufferStruct>>
    {
    private: 
        using Super = std::shared_ptr<TUniformBuffer<UniformBufferStruct>>;
    public:
        TUniformBufferRef()
            : Super(nullptr)
        { }
        TUniformBufferRef(TUniformBuffer<UniformBufferStruct> *Ptr)
            : Super(Ptr)
        { }
        TUniformBufferRef(const Super &Ptr)
            : Super(Ptr)
        { }
        TUniformBufferRef(Super &&Ptr)
            : Super(std::move(Ptr))
        { }
    };

    template<class UniformBufferStruct>
    inline TUniformBufferRef<UniformBufferStruct> CreateUniformBuffer()
    {
        return TUniformBufferRef<UniformBufferStruct>(std::make_shared<TUniformBuffer<UniformBufferStruct>>());
    }

    template <class T>
    EUniformBufferMemberType TranslateToMemberType()
    {
        if constexpr (std::is_same<T, bool>::value)
            return EUniformBufferMemberType::UBMT_bool;

        else if constexpr (std::is_same<T, int32>::value)
            return EUniformBufferMemberType::UBMT_int;

        else if constexpr (std::is_same<T, uint32>::value)
            return EUniformBufferMemberType::UBMT_uint;

        else if constexpr (std::is_same<T, float>::value)
            return EUniformBufferMemberType::UBMT_float;

        else if constexpr (std::is_same<T, glm::vec2>::value)
            return EUniformBufferMemberType::UBMT_vec2;

        else if constexpr (std::is_same<T, glm::vec3>::value)
            return EUniformBufferMemberType::UBMT_vec3;
            
        else if constexpr (std::is_same<T, glm::vec4>::value)
            return EUniformBufferMemberType::UBMT_vec4;

        else if constexpr (std::is_same<T, glm::bvec2>::value)
            return EUniformBufferMemberType::UBMT_bvec2;

        else if constexpr (std::is_same<T, glm::bvec3>::value)
            return EUniformBufferMemberType::UBMT_bvec3;

        else if constexpr (std::is_same<T, glm::bvec4>::value)
            return EUniformBufferMemberType::UBMT_bvec4;

        else if constexpr (std::is_same<T, glm::ivec2>::value)
            return EUniformBufferMemberType::UBMT_ivec2;

        else if constexpr (std::is_same<T, glm::ivec3>::value)
            return EUniformBufferMemberType::UBMT_ivec3;

        else if constexpr (std::is_same<T, glm::ivec4>::value)
            return EUniformBufferMemberType::UBMT_ivec4;

        else if constexpr (std::is_same<T, glm::mat2>::value)
            return EUniformBufferMemberType::UBMT_mat2;

        else if constexpr (std::is_same<T, glm::mat3>::value)
            return EUniformBufferMemberType::UBMT_mat3;

        else if constexpr (std::is_same<T, glm::mat4>::value)
            return EUniformBufferMemberType::UBMT_mat4;

        else 
            return EUniformBufferMemberType::UBMT_Null;
    }

    // template <class MemberType>
    // struct CorrespondingEnum {
    //     static EUniformBufferMemberType Enum() { return EUniformBufferMemberType::UBMT_Null; }
    // };
    
    // template <>
    // struct CorrespondingEnum<bool> { 
    //     static EUniformBufferMemberType Enum() { return EUniformBufferMemberType::UBMT_bool; } 
    // };
    
    // template <>
    // struct CorrespondingEnum<int32> { 
    //     static EUniformBufferMemberType Enum() { return EUniformBufferMemberType::UBMT_int; } 
    // };
    
    // template <>
    // struct CorrespondingEnum<uint32> { 
    //     static EUniformBufferMemberType Enum() { return EUniformBufferMemberType::UBMT_uint; } 
    // };
    
    // template <>
    // struct CorrespondingEnum<float> { 
    //     static EUniformBufferMemberType Enum() { return EUniformBufferMemberType::UBMT_float; } 
    // };
    
    // template <>
    // struct CorrespondingEnum<glm::vec2> { 
    //     static EUniformBufferMemberType Enum() { return EUniformBufferMemberType::UBMT_vec2; } 
    // };
    
    // template <>
    // struct CorrespondingEnum<glm::vec3> { 
    //     static EUniformBufferMemberType Enum() { return EUniformBufferMemberType::UBMT_vec3; } 
    // };
    
    // template <>
    // struct CorrespondingEnum<glm::vec4> { 
    //     static EUniformBufferMemberType Enum() { return EUniformBufferMemberType::UBMT_vec4; } 
    // };
    
    // template <>
    // struct CorrespondingEnum<glm::mat2> { 
    //     static EUniformBufferMemberType Enum() { return EUniformBufferMemberType::UBMT_mat2; } 
    // };
    
    // template <>
    // struct CorrespondingEnum<glm::mat3> { 
    //     static EUniformBufferMemberType Enum() { return EUniformBufferMemberType::UBMT_mat3; } 
    // };
    
    // template <>
    // struct CorrespondingEnum<glm::mat4> { 
    //     static EUniformBufferMemberType Enum() { return EUniformBufferMemberType::UBMT_mat4; } 
    // };

    // struct NormalVersionTag {};
    // template<class T> struct TagDispatchTrait { 
    //     using Tag = NormalVersionTag; 
    // };

    // struct mat2PartialVersionTag {};
    // template<> 
    // struct TagDispatchTrait<glm::mat2> { 
    //     using Tag = mat2PartialVersionTag; 
    // };

    // struct mat3PartialVersionTag {};
    // template<> 
    // struct TagDispatchTrait<glm::mat3> { 
    //     using Tag = mat3PartialVersionTag; 
    // };

    // struct mat4PartialVersionTag {};
    // template<> 
    // struct TagDispatchTrait<glm::mat4> { 
    //     using Tag = mat4PartialVersionTag; 
    // };
        


    // #define INTERNAL_SET_MAT(N) \
    //     template<class T> \
    //     void InternalSet(const nilou::FUniformBufferMemberDeclaration &MemberDeclaration, uint8 *Data, const T &Value, mat##N##PartialVersionTag) \
    //     { \
    //         auto BaseOffset = Data + MemberDeclaration.Offset; \
    //         glm::mat##N::col_type *pointer; \
    //         for (int i = 0; i < N; i++) \
    //         { \
    //             pointer = (glm::mat##N::col_type *)(BaseOffset + 16*i); \
    //             *pointer = Value[i]; \
    //         } \
    //     }
    // template<class T>
    // void InternalSet(const nilou::FUniformBufferMemberDeclaration &MemberDeclaration, uint8 *Data, const T &Value, NormalVersionTag)
    // {
    //     T *pointer = (T *)(Data + MemberDeclaration.Offset);
    //     *pointer = Value;
    // }
    // INTERNAL_SET_MAT(2)
    // INTERNAL_SET_MAT(3)
    // INTERNAL_SET_MAT(4)

    // #undef INTERNAL_SET_MAT


    // #define INTERNAL_GET_MAT(N) \
    // template<class T> \
    // void InternalGet(const nilou::FUniformBufferMemberDeclaration &MemberDeclaration, uint8 *Data, T *OutValue, mat##N##PartialVersionTag) \
    // { \
    //     auto BaseOffset = Data + MemberDeclaration.Offset; \
    //     glm::mat##N::col_type *pointer; \
    //     for (int i = 0; i < N; i++) \
    //     { \
    //         pointer = (glm::mat##N::col_type *)(BaseOffset + 16*i); \
    //         (*OutValue)[i] = *pointer; \
    //     } \
    // }
    // template<class T>
    // void InternalGet(const nilou::FUniformBufferMemberDeclaration &MemberDeclaration, uint8 *Data, T *OutValue, NormalVersionTag)
    // {
    //     const T *pointer = (const T *)(Data + MemberDeclaration.Offset);
    //     *OutValue = *pointer;
    // }
    // INTERNAL_GET_MAT(2)
    // INTERNAL_GET_MAT(3)
    // INTERNAL_GET_MAT(4)

    // #undef INTERNAL_GET_MAT

    inline size_t find_first_not_delim(const std::string &s, char delim, size_t pos)
    {
        for (size_t i = pos; i < s.size(); i++)
            if (s[i] != delim)
                return i;
        return std::string::npos;
    }
    inline std::vector<std::string> split(const std::string &s, char delim = ' ')
    {
        std::vector<std::string> tokens;
        size_t lastPos = find_first_not_delim(s, delim, 0);
        size_t pos = s.find(delim, lastPos);
        while (lastPos != std::string::npos)
        {
            tokens.push_back(s.substr(lastPos, pos - lastPos));
            lastPos = find_first_not_delim(s, delim, pos);
            pos = s.find(delim, lastPos);
        }
        return tokens;
    }

    inline void trim(std::string &s)
    {
        if( !s.empty() )
        {
            s.erase(0, s.find_first_not_of(" "));
            s.erase(s.find_last_not_of(" ") + 1);
        }
    }

    template<class T>
    void FUniformBuffer::Set(const std::string &Name, const T &Value)
    {
        EUniformBufferMemberType MemberType;
        uint32 Offset = ComputeOffsetAndMemberType(Name, &MemberType);
        check(TranslateToMemberType<T>() == MemberType);
        if constexpr (std::is_same<T, glm::mat2>::value || std::is_same<T, glm::mat3>::value || std::is_same<T, glm::mat4>::value)
        {
            for (int i = 0; i < TShaderParameterTypeInfo<T>::NumColumns; i++)
            {
                using col_type = T::col_type;
                col_type *pointer = (col_type *)(this->Data + Offset + i * TShaderParameterTypeInfo<T>::Alignment);
                *pointer = Value[i];
            }
        }
        // else if constexpr (std::is_same<T, glm::mat4>::value)
        // {
        //     T *pointer = (T *)(this->Data + Offset);
        //     *pointer = Value;
        // }
        else 
        {
            T *pointer = (T *)(this->Data + Offset);
            *pointer = Value;
        }

        // InternalSet(MemberDeclaration, this->Data, Value, typename TagDispatchTrait<T>::Tag {});
    }
    template<class T>
    void FUniformBuffer::Get(const std::string &Name, T *OutValue)
    {
        EUniformBufferMemberType MemberType;
        uint32 Offset = ComputeOffsetAndMemberType(Name, &MemberType);
        check(TranslateToMemberType<T>() == MemberType);
        if constexpr (std::is_same<T, glm::mat2>::value || std::is_same<T, glm::mat3>::value || std::is_same<T, glm::mat4>::value)
        {
            for (int i = 0; i < TShaderParameterTypeInfo<T>::NumRows; i++)
            {
                using row_type = T::row_type;
                const row_type *pointer = (const row_type *)(this->Data + Offset + i * TShaderParameterTypeInfo<T>::Alignment);
                (*OutValue)[i] = *pointer;
            }
        }
        else 
        {
            const T *pointer = (T *)(this->Data + Offset);
            *OutValue = *pointer;
        }
        // auto MemberDeclaration = Declaration.GetMemberDeclaration(Name);
        // check(CorrespondingEnum<T>::Enum() == MemberDeclaration.MemberType);
        // InternalGet(MemberDeclaration, this->Data, OutValue, typename TagDispatchTrait<T>::Tag {});
    }

    // template<class T>
    // void FUniformBuffer::Set(const std::string &Name, uint32 ArrayIndex, const T &Value)
    // {
    //     auto &MemberDeclaration = Declaration.GetMemberDeclaration(Name);
    //     check(CorrespondingEnum<T>::Enum() == MemberDeclaration.MemberType);
    //     T *pointer = (T *)(Data + MemberDeclaration.Offset + 16 * ArrayIndex);
    //     *pointer = Value;
    // }
    // template<class T>
    // void FUniformBuffer::Get(const std::string &Name, uint32 ArrayIndex, T *OutValue)
    // {
    //     auto &MemberDeclaration = Declaration.GetMemberDeclaration(Name);
    //     check(CorrespondingEnum<T>::Enum() == MemberDeclaration.MemberType);
    //     const T *pointer = (const T *)(Data + MemberDeclaration.Offset + 16 * ArrayIndex);
    //     *OutValue = *pointer;
    // }
}