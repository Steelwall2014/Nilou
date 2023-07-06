#include <memory>
#include <UDRefl/UDRefl.hpp>
#include "DynamicRHI.h"
#include "RHIDefinitions.h"
#include "UniformBuffer.h"

using namespace Ubpa;
using namespace Ubpa::UDRefl;

namespace nilou {

    FDynamicUniformBuffer::FDynamicUniformBuffer(const std::string& InStructName)
    {
        UpdateDataType(InStructName);
    }

    FDynamicUniformBuffer::FDynamicUniformBuffer(const FDynamicUniformBuffer& Other)
    {
        UpdateDataType(Other.StructName);
        memcpy(Data, Other.Data, Size);
    }

    void FDynamicUniformBuffer::SetScalarParameterValue(std::string_view ParamName, float Value)
    {
        ObjectView Obj{Type(StructName), Data};
        Obj.Var(Name(ParamName)) = Value;
    }

    void FDynamicUniformBuffer::UpdateDataType(std::string_view InStructName)
    {
        if (!InStructName.empty())
        {
            StructName = InStructName;
            Type type = Type(StructName);
            TypeInfo* info = Mngr.GetTypeInfo(type);
            ObjectView Obj = Mngr.New(type);
            Data = Obj.GetPtr();
            Size = info->size;
        }
    }

    /** Begin FRenderResource Interface */
    void FDynamicUniformBuffer::InitRHI()
    {
        if (StructName != "")
        {
            FRenderResource::InitRHI();
            UniformBufferRHI = FDynamicRHI::GetDynamicRHI()->RHICreateUniformBuffer(Size, Usage, Data);
        }
    }
    void FDynamicUniformBuffer::ReleaseRHI()
    {
        UniformBufferRHI = nullptr;
        FRenderResource::ReleaseRHI();
    }
    /** End FRenderResource Interface */

    void FDynamicUniformBuffer::UpdateUniformBuffer()
    {
        FDynamicRHI::GetDynamicRHI()->RHIUpdateUniformBuffer(UniformBufferRHI, Data);
    }

    void FDynamicUniformBuffer::Serialize(FArchive& Ar)
    {
        nlohmann::json& json = Ar.Node;
        json["StructName"] = StructName;
        FArchive local_Ar(json["Data"], Ar);
        ObjectView Obj(Type(StructName), Data);
        Obj.Invoke("Serialize", TempArgsView{ local_Ar });
    }

    void FDynamicUniformBuffer::Deserialize(FArchive& Ar)
    {
        nlohmann::json& json = Ar.Node;
        if (json.contains("StructName"))
        { 
            StructName = json["StructName"].get<std::string>();
            Type type = Type(StructName);
            TypeInfo* info = Mngr.GetTypeInfo(type);
            ObjectView Obj = Mngr.New(type);
            Data = Obj.GetPtr();
            Size = info->size;
            if (json.contains("Data"))
            {
                FArchive local_Ar(json["Data"], Ar);
                Obj.Invoke("Deserialize", TempArgsView{ local_Ar });
            }
        }
    }

}