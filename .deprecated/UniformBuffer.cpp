#include <memory>
#include "DynamicRHI.h"
#include "RHIDefinitions.h"
#include "UniformBuffer.h"

    
namespace nilou {
    // FUniformBufferBuilder FUniformBufferBuilder::GlobalBuilder;
    // FUniformBufferStructDeclaration FUniformBufferBuilder::CurrentStructDeclaration = FUniformBufferStructDeclaration();
    // uint32 FUniformBufferBuilder::CurrentOffset = 0;
    FUniformBufferStructDeclaration &FUniformBufferBuilder::GetCurrentStructDeclaration()
    {
        return GetUniformBufferBuilder()->CurrentStructDeclaration;
    }
    uint32 &FUniformBufferBuilder::GetCurrentOffset()
    {
        return GetUniformBufferBuilder()->CurrentOffset;
    }
    FUniformBufferBuilder *FUniformBufferBuilder::GetUniformBufferBuilder()
    {
        static FUniformBufferBuilder *Builder = new FUniformBufferBuilder;
        return Builder;
    }
}
    // template<class T>
    // void InternalSet(const std::string &Name, const T &Value, NormalVersionTag);
    // template<class T>
    // void InternalSet(const std::string &Name, const T &Value, VERSION_TAG(mat2));
    // template<class T>
    // void InternalSet(const std::string &Name, const T &Value, VERSION_TAG(mat3));
    // template<class T>
    // void InternalSet(const std::string &Name, const T &Value, VERSION_TAG(mat4));

    // template<class T>
    // void InternalGet(const std::string &Name, T &OutValue, NormalVersionTag);
    // template<class T>
    // void InternalGet(const std::string &Name, T &OutValue, VERSION_TAG(mat2));
    // template<class T>
    // void InternalGet(const std::string &Name, T &OutValue, VERSION_TAG(mat3));
    // template<class T>
    // void InternalGet(const std::string &Name, T &OutValue, VERSION_TAG(mat4));


// template<class T>
// void InternalSet(const nilou::FUniformBuffer &Buffer, const std::string &Name, const T &Value, VERSION_TAG(mat2))
// {
//     auto &MemberDeclaration = Buffer.GetStructDeclaration().GetMemberDeclaration(Name);
//     SET_MAT(2, Value, Buffer.GetData() + MemberDeclaration.Offset)
//     // SetMat<2>(MemberDeclaration, &Value, Data + MemberDeclaration.Offset);
//     // glm::mat2::col_type *pointer = (glm::mat2::col_type *)(Data + MemberDeclaration.Offset);
//     // *pointer = Value[0];
//     // pointer = (glm::mat2::col_type *)(Data + MemberDeclaration.Offset + 16);
//     // *pointer = Value[1];
// }
// template<class T>
// void InternalSet(const nilou::FUniformBuffer &Buffer, const std::string &Name, const T &Value, VERSION_TAG(mat3))
// {
//     auto &MemberDeclaration = Buffer.GetStructDeclaration().GetMemberDeclaration(Name);
//     SET_MAT(3, Value, Buffer.GetData() + MemberDeclaration.Offset)
//     // glm::mat3::col_type *pointer = (glm::mat3::col_type *)(Data + MemberDeclaration.Offset);
//     // *pointer = Value[0];
//     // pointer = (glm::mat3::col_type *)(Data + MemberDeclaration.Offset + 16);
//     // *pointer = Value[1];
//     // pointer = (glm::mat3::col_type *)(Data + MemberDeclaration.Offset + 32);
//     // *pointer = Value[2];
// }
// template<class T>
// void InternalSet(const nilou::FUniformBuffer &Buffer, const std::string &Name, const T &Value, VERSION_TAG(mat4))
// {
//     auto &MemberDeclaration = Buffer.GetStructDeclaration().GetMemberDeclaration(Name);
//     SET_MAT(4, Value, Buffer.GetData() + MemberDeclaration.Offset)
//     // glm::mat4::col_type *pointer = (glm::mat4::col_type *)(Data + MemberDeclaration.Offset);
//     // *pointer = Value[0];
//     // pointer = (glm::mat4::col_type *)(Data + MemberDeclaration.Offset + 16);
//     // *pointer = Value[1];
//     // pointer = (glm::mat4::col_type *)(Data + MemberDeclaration.Offset + 32);
//     // *pointer = Value[2];
//     // pointer = (glm::mat4::col_type *)(Data + MemberDeclaration.Offset + 48);
//     // *pointer = Value[3];
// }


// template<class T>
// void InternalGet(const nilou::FUniformBuffer &Buffer, const std::string &Name, T &OutValue, NormalVersionTag)
// {
//     auto &MemberDeclaration = Buffer.GetStructDeclaration().GetMemberDeclaration(Name);
//     T *pointer = (T *)(Buffer.GetData() + MemberDeclaration.Offset);
//     OutValue = *pointer;
// }
// template<class T>
// void InternalGet(const nilou::FUniformBuffer &Buffer, const std::string &Name, T &OutValue, VERSION_TAG(mat2))
// {
//     auto &MemberDeclaration = Buffer.GetStructDeclaration().GetMemberDeclaration(Name);
//     GET_MAT(2, OutValue, Buffer.GetData() + MemberDeclaration.Offset)
//     // glm::mat2::col_type *pointer = (glm::mat2::col_type *)(Data + MemberDeclaration.Offset);
//     // OutValue[0] = *pointer;
//     // pointer = (glm::mat2::col_type *)(Data + MemberDeclaration.Offset + 16);
//     // OutValue[1] = *pointer;
// }
// template<class T>
// void InternalGet(const nilou::FUniformBuffer &Buffer, const std::string &Name, T &OutValue, VERSION_TAG(mat3))
// {
//     auto &MemberDeclaration = Buffer.GetStructDeclaration().GetMemberDeclaration(Name);
//     GET_MAT(3, OutValue, Buffer.GetData() + MemberDeclaration.Offset)
//     // glm::mat3::col_type *pointer = (glm::mat3::col_type *)(Data + MemberDeclaration.Offset);
//     // OutValue[0] = *pointer;
//     // pointer = (glm::mat3::col_type *)(Data + MemberDeclaration.Offset + 16);
//     // OutValue[1] = *pointer;
//     // pointer = (glm::mat3::col_type *)(Data + MemberDeclaration.Offset + 32);
//     // OutValue[2] = *pointer;
// }
// template<class T>
// void InternalGet(const nilou::FUniformBuffer &Buffer, const std::string &Name, T &OutValue, VERSION_TAG(mat4))
// {
//     auto &MemberDeclaration = Buffer.GetStructDeclaration().GetMemberDeclaration(Name);
//     GET_MAT(4, OutValue, Buffer.GetData() + MemberDeclaration.Offset)
//     // glm::mat4::col_type *pointer = (glm::mat4::col_type *)(Data + MemberDeclaration.Offset);
//     // OutValue[0] = *pointer;
//     // pointer = (glm::mat4::col_type *)(Data + MemberDeclaration.Offset + 16);
//     // OutValue[1] = *pointer;
//     // pointer = (glm::mat4::col_type *)(Data + MemberDeclaration.Offset + 32);
//     // OutValue[2] = *pointer;
//     // pointer = (glm::mat4::col_type *)(Data + MemberDeclaration.Offset + 48);
//     // OutValue[3] = *pointer;
// }

namespace nilou {
    std::map<std::string, FUniformBufferStructDeclaration> &FUniformBuffer::GetGlobalStructDeclarations()
    {
        static auto GlobalStructDeclarations = std::map<std::string, FUniformBufferStructDeclaration>();
        return GlobalStructDeclarations;
    }

    void FUniformBufferBuilder::BeginUniformBufferStruct(std::string StructName)
    {
        // FUniformBufferBuilder *This_ = &FUniformBufferBuilder::GlobalBuilder;
        GetCurrentStructDeclaration().MemberDeclarations.clear();
        GetCurrentStructDeclaration().StructName = StructName;
        GetCurrentStructDeclaration().StructSize = 0;
        GetCurrentOffset() = 0;
    }

    void FUniformBufferBuilder::DeclareUniformBufferStructMember(const std::string &NestedStructName, const std::string &Name)
    {
        DeclareUniformBufferStructMemberInternal(NestedStructName, Name, 0);
    }   

    void FUniformBufferBuilder::DeclareUniformBufferStructMember(const std::string &NestedStructName, int32 NumElements, const std::string &Name)
    {
        DeclareUniformBufferStructMemberInternal(NestedStructName, Name, NumElements);
    }   

    FUniformBufferStructDeclaration &FUniformBufferBuilder::EndUniformBufferStruct()
    {
        // FUniformBufferBuilder *This_ = &FUniformBufferBuilder::GlobalBuilder;
        GetCurrentStructDeclaration().StructSize = GetCurrentOffset();
        FUniformBuffer::GetGlobalStructDeclarations()[GetCurrentStructDeclaration().StructName] = GetCurrentStructDeclaration();
        return FUniformBuffer::GetGlobalStructDeclarations()[GetCurrentStructDeclaration().StructName];
    }

    void FUniformBufferBuilder::DeclareUniformBufferStructMemberInternal(const std::string &NestedStructName, const std::string &Name, int32 NumElements)
    {
        // FUniformBufferBuilder *This_ = &FUniformBufferBuilder::GlobalBuilder;
        FUniformBufferMemberDeclaration MemberDeclaration;
        MemberDeclaration.MemberType = EUniformBufferMemberType::UBMT_struct;
        MemberDeclaration.Name = Name;
        MemberDeclaration.NumElements = NumElements;

        const FUniformBufferStructDeclaration &NestedStruct = FUniformBuffer::GetStructDeclarationByName(NestedStructName);
        MemberDeclaration.NestedStructDeclaration = &NestedStruct;
        uint32 BaseAlignment = NestedStruct.GetBaseAlignment();
        
        MemberDeclaration.Offset = GetCurrentOffset() % BaseAlignment == 0 ?
                                    GetCurrentOffset() :
                                    (GetCurrentOffset() / BaseAlignment + 1) * BaseAlignment;

        GetCurrentOffset() = MemberDeclaration.Offset + BaseAlignment * std::max(1, NumElements);
        GetCurrentStructDeclaration().MemberDeclarations[Name] = MemberDeclaration;

    }


    FUniformBuffer::FUniformBuffer()
    {
    }

    FUniformBuffer::FUniformBuffer(const FUniformBufferStructDeclaration &InDeclaration)
    {
        Declaration = InDeclaration;
        Size = Declaration.StructSize;
        Data = new uint8[Size];
    }
    
    // void FUniformBuffer::Init(const FUniformBufferStructDeclaration &InDeclaration)
    // {
    //     Declaration = InDeclaration;
    //     Size = Declaration.StructSize;
    //     Data = new uint8[Size];
    // }
    
    // void FUniformBuffer::InitWithStruct(const std::string &InStructName)
    // {
    //     Declaration = GetStructDeclarationByName(InStructName);
    //     Size = Declaration.StructSize;
    //     Data = new uint8[Size];
    // }
    
    void FUniformBuffer::InitRHI()
    {
        FRenderResource::InitRHI();
        UniformBufferRHI = GDynamicRHI->RHICreateUniformBuffer(Size, Usage, Data);
    }
    
    void FUniformBuffer::UpdateUniformBuffer()
    {
        GDynamicRHI->RHIUpdateUniformBuffer(UniformBufferRHI, Data);
    }

    uint32 FUniformBuffer::ComputeOffsetAndMemberType(const std::string &Name, EUniformBufferMemberType *MemberType)
    {
        const FUniformBufferStructDeclaration *CurrDeclaration = &this->Declaration;
        std::vector<std::string> tokens = split(Name, '.');
        uint32 Offset = 0;
        for (std::string &token : tokens)
        {
            std::string str_Name, str_ElementIndex;
            trim(token);
            std::string *target = &str_Name;
            for (int i = 0; i < token.size(); i++)
            {
                if (token[i] == '[')
                {
                    target = &str_ElementIndex;
                    continue;
                }
                else if (token[i] == ']')
                    break;
                target->push_back(token[i]);
            }
            int32 ElementIndex = str_ElementIndex.empty() ? 0 : atoi(str_ElementIndex.c_str());

            const FUniformBufferMemberDeclaration &MemberDeclaration = CurrDeclaration->GetMemberDeclaration(str_Name);

            /** meaning that this member is an array so we need to check whether the index is within the range */
            if (MemberDeclaration.NumElements > 0)  
                check(0 <= ElementIndex && ElementIndex < MemberDeclaration.NumElements);
                
            if (MemberDeclaration.MemberType == EUniformBufferMemberType::UBMT_struct)
            {
                Offset += MemberDeclaration.Offset + MemberDeclaration.NestedStructDeclaration->GetBaseAlignment() * ElementIndex;
                CurrDeclaration = MemberDeclaration.NestedStructDeclaration;
            }
            else 
            {
                Offset += MemberDeclaration.Offset + SHADER_PARAMETER_ARRAY_ELEMENT_ALIGNMENT * ElementIndex;
            }

            *MemberType = MemberDeclaration.MemberType;
        }
        // *OutMemberDeclaration = *CurrMemberDeclaration;
        if (*MemberType == EUniformBufferMemberType::UBMT_struct)
            std::cout << "CRITICAL WARNING: It is not implemented to assign value directly to a struct" << std::endl;
        return Offset;
    }

    
    // void FUniformBuffer::Set(std::string Name, bool Value)
    // {
    //     auto &MemberDeclaration = Declaration.GetMemberDeclaration(Name);
    //     bool *pointer = (bool *)(Data + MemberDeclaration.Offset);
    //     *pointer = Value;
    // }
    // void FUniformBuffer::Set(std::string Name, int32 Value)
    // {
    //     auto &MemberDeclaration = Declaration.GetMemberDeclaration(Name);
    //     int32 *pointer = (int32 *)(Data + MemberDeclaration.Offset);
    //     *pointer = Value;
    // }
    // void FUniformBuffer::Set(std::string Name, uint32 Value)
    // {
    //     auto &MemberDeclaration = Declaration.GetMemberDeclaration(Name);
    //     uint32 *pointer = (uint32 *)(Data + MemberDeclaration.Offset);
    //     *pointer = Value;
    // }
    // void FUniformBuffer::Set(std::string Name, float Value)
    // {
    //     auto &MemberDeclaration = Declaration.GetMemberDeclaration(Name);
    //     float *pointer = (float *)(Data + MemberDeclaration.Offset);
    //     *pointer = Value;
    // }
    // void FUniformBuffer::Set(std::string Name, const glm::vec2 &Value)
    // {
    //     auto &MemberDeclaration = Declaration.GetMemberDeclaration(Name);
    //     glm::vec2 *pointer = (glm::vec2 *)(Data + MemberDeclaration.Offset);
    //     *pointer = Value;
    // }
    // void FUniformBuffer::Set(std::string Name, const glm::vec3 &Value)
    // {
    //     auto &MemberDeclaration = Declaration.GetMemberDeclaration(Name);
    //     glm::vec3 *pointer = (glm::vec3 *)(Data + MemberDeclaration.Offset);
    //     *pointer = Value;
    // }
    // void FUniformBuffer::Set(std::string Name, const glm::vec4 &Value)
    // {
    //     auto &MemberDeclaration = Declaration.GetMemberDeclaration(Name);
    //     glm::vec4 *pointer = (glm::vec4 *)(Data + MemberDeclaration.Offset);
    //     *pointer = Value;
    // }
    // void FUniformBuffer::Set(std::string Name, const glm::mat2 &Value)
    // {
    //     auto &MemberDeclaration = Declaration.GetMemberDeclaration(Name);
    //     glm::mat2::col_type *pointer = (glm::mat2::col_type *)(Data + MemberDeclaration.Offset);
    //     *pointer = Value[0];
    //     pointer = (glm::mat2::col_type *)(Data + MemberDeclaration.Offset + 16);
    //     *pointer = Value[1];
    // }
    // void FUniformBuffer::Set(std::string Name, const glm::mat3 &Value)
    // {
    //     auto &MemberDeclaration = Declaration.GetMemberDeclaration(Name);
    //     glm::mat3::col_type *pointer = (glm::mat3::col_type *)(Data + MemberDeclaration.Offset);
    //     *pointer = Value[0];
    //     pointer = (glm::mat3::col_type *)(Data + MemberDeclaration.Offset + 16);
    //     *pointer = Value[1];
    //     pointer = (glm::mat3::col_type *)(Data + MemberDeclaration.Offset + 32);
    //     *pointer = Value[2];
    // }
    // void FUniformBuffer::Set(std::string Name, const glm::mat4 &Value)
    // {
    //     auto &MemberDeclaration = Declaration.GetMemberDeclaration(Name);
    //     glm::mat4::col_type *pointer = (glm::mat4::col_type *)(Data + MemberDeclaration.Offset);
    //     *pointer = Value[0];
    //     pointer = (glm::mat4::col_type *)(Data + MemberDeclaration.Offset + 16);
    //     *pointer = Value[1];
    //     pointer = (glm::mat4::col_type *)(Data + MemberDeclaration.Offset + 32);
    //     *pointer = Value[2];
    //     pointer = (glm::mat4::col_type *)(Data + MemberDeclaration.Offset + 48);
    //     *pointer = Value[3];
    // }

    // template<class T>
    // void FUniformBuffer::Set(const std::string &Name, const T &Value)
    // {
    //     InternalSet(*this, Name, Value, typename TagDispatchTrait<T>::Tag {});
    // }
    // template<class T>
    // void FUniformBuffer::Get(const std::string &Name, T &OutValue)
    // {
    //     InternalGet(*this, Name, OutValue, typename TagDispatchTrait<T>::Tag {});
    // }

    // template<class T>
    // void FUniformBuffer::Set(const std::string &Name, uint32 ArrayIndex, const T &Value)
    // {
    //     auto &MemberDeclaration = Declaration.GetMemberDeclaration(Name);
    //     T *pointer = (T *)(Data + MemberDeclaration.Offset + 16 * ArrayIndex);
    //     *pointer = Value;
    // }
    // template<class T>
    // void FUniformBuffer::Get(const std::string &Name, uint32 ArrayIndex, T &OutValue)
    // {
    //     auto &MemberDeclaration = Declaration.GetMemberDeclaration(Name);
    //     const T *pointer = (const T *)(Data + MemberDeclaration.Offset + 16 * ArrayIndex);
    //     OutValue = *pointer;
    // }


    // void FUniformBuffer::Get(std::string Name, bool &OutValue)
    // {
    //     auto &MemberDeclaration = Declaration.GetMemberDeclaration(Name);
    //     bool *pointer = (bool *)(Data + MemberDeclaration.Offset);
    //     OutValue = *pointer;
    // }
    // void FUniformBuffer::Get(std::string Name, int32 &OutValue)
    // {
    //     auto &MemberDeclaration = Declaration.GetMemberDeclaration(Name);
    //     int32 *pointer = (int32 *)(Data + MemberDeclaration.Offset);
    //     OutValue = *pointer;
    // }
    // void FUniformBuffer::Get(std::string Name, uint32 &OutValue)
    // {
    //     auto &MemberDeclaration = Declaration.GetMemberDeclaration(Name);
    //     uint32 *pointer = (uint32 *)(Data + MemberDeclaration.Offset);
    //     OutValue = *pointer;
    // }
    // void FUniformBuffer::Get(std::string Name, float &OutValue)
    // {
    //     auto &MemberDeclaration = Declaration.GetMemberDeclaration(Name);
    //     float *pointer = (float *)(Data + MemberDeclaration.Offset);
    //     OutValue = *pointer;
    // }
    // void FUniformBuffer::Get(std::string Name, double &OutValue)
    // {
    //     auto &MemberDeclaration = Declaration.GetMemberDeclaration(Name);
    //     double *pointer = (double *)(Data + MemberDeclaration.Offset);
    //     OutValue = *pointer;
    // }
    // void FUniformBuffer::Get(std::string Name, glm::vec2 &OutValue)
    // {
    //     auto &MemberDeclaration = Declaration.GetMemberDeclaration(Name);
    //     glm::vec2 *pointer = (glm::vec2 *)(Data + MemberDeclaration.Offset);
    //     OutValue = *pointer;
    // }
    // void FUniformBuffer::Get(std::string Name, glm::vec3 &OutValue)
    // {
    //     auto &MemberDeclaration = Declaration.GetMemberDeclaration(Name);
    //     glm::vec3 *pointer = (glm::vec3 *)(Data + MemberDeclaration.Offset);
    //     OutValue = *pointer;
    // }
    // void FUniformBuffer::Get(std::string Name, glm::vec4 &OutValue)
    // {
    //     auto &MemberDeclaration = Declaration.GetMemberDeclaration(Name);
    //     glm::vec4 *pointer = (glm::vec4 *)(Data + MemberDeclaration.Offset);
    //     OutValue = *pointer;
    // }
    // void FUniformBuffer::Get(std::string Name, glm::mat2 &OutValue)
    // {
    //     auto &MemberDeclaration = Declaration.GetMemberDeclaration(Name);
    //     glm::mat2::col_type *pointer = (glm::mat2::col_type *)(Data + MemberDeclaration.Offset);
    //     OutValue[0] = *pointer;
    //     pointer = (glm::mat2::col_type *)(Data + MemberDeclaration.Offset + 16);
    //     OutValue[1] = *pointer;
    // }
    // void FUniformBuffer::Get(std::string Name, glm::mat3 &OutValue)
    // {
    //     auto &MemberDeclaration = Declaration.GetMemberDeclaration(Name);
    //     glm::mat3::col_type *pointer = (glm::mat3::col_type *)(Data + MemberDeclaration.Offset);
    //     OutValue[0] = *pointer;
    //     pointer = (glm::mat3::col_type *)(Data + MemberDeclaration.Offset + 16);
    //     OutValue[1] = *pointer;
    //     pointer = (glm::mat3::col_type *)(Data + MemberDeclaration.Offset + 32);
    //     OutValue[2] = *pointer;
    // }
    // void FUniformBuffer::Get(std::string Name, glm::mat4 &OutValue)
    // {
    //     auto &MemberDeclaration = Declaration.GetMemberDeclaration(Name);
    //     glm::mat4::col_type *pointer = (glm::mat4::col_type *)(Data + MemberDeclaration.Offset);
    //     OutValue[0] = *pointer;
    //     pointer = (glm::mat4::col_type *)(Data + MemberDeclaration.Offset + 16);
    //     OutValue[1] = *pointer;
    //     pointer = (glm::mat4::col_type *)(Data + MemberDeclaration.Offset + 32);
    //     OutValue[2] = *pointer;
    //     pointer = (glm::mat4::col_type *)(Data + MemberDeclaration.Offset + 48);
    //     OutValue[3] = *pointer;
    // }

    // template<int N>
    // void FUniformBuffer::SetMat(const FUniformBufferMemberDeclaration &MemberDeclaration, const void *Value, unsigned char *BaseOffset)
    // {
    //     glm::mat4::col_type *pointer = (glm::mat4::col_type *)(BaseOffset + 16*N);
    //     *pointer = (*static_cast<const glm::mat4 *>(Value))[N];
    //     SetMat<N-1>(MemberDeclaration, Value, BaseOffset);
    // }
    // template<>
    // void FUniformBuffer::SetMat<0>(const FUniformBufferMemberDeclaration &MemberDeclaration, const void *Value, unsigned char *BaseOffset)
    // {
    //     glm::mat4::col_type *pointer = (glm::mat4::col_type *)(BaseOffset + 16*0);
    //     *pointer = (*(const glm::mat4 * const)Value)[0];
    // }

    // template<int N>
    // void FUniformBuffer::GetMat(const FUniformBufferMemberDeclaration &MemberDeclaration, void *Value, unsigned char *BaseOffset)
    // {
    //     glm::mat4::col_type *pointer = (glm::mat4::col_type *)(BaseOffset + 16*N);
    //     (*static_cast<glm::mat4 *>(Value))[N] = *pointer;
    //     GetMat<N-1>(MemberDeclaration, Value, BaseOffset);
    // }
    // template<>
    // void FUniformBuffer::GetMat<0>(const FUniformBufferMemberDeclaration &MemberDeclaration, void *Value, unsigned char *BaseOffset)
    // {
    //     glm::mat4::col_type *pointer = (glm::mat4::col_type *)(BaseOffset + 16*0);
    //     (*static_cast<glm::mat4 *>(Value))[0] = *pointer;
    // }


}