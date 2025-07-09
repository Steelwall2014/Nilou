#pragma once

#include <memory>
#include <vector>
#include <shaderc/shaderc.h>
#include "RHIResources.h"
#include "ShaderParameter.h"
#include "Shader.h"
#include "RenderResource.h"

namespace nilou {
    class FShaderInstance
    {
    public:

        FShaderInstance(
            const std::string& InShaderName, 
            const std::string& InCode, 
            EShaderStage InShaderStage,
            EShaderMetaType InShaderMetaType)
            : ShaderName(InShaderName)
            , Code(InCode)
            , ShaderStage(InShaderStage)
            , ShaderMetaType(InShaderMetaType)
        { }

        RHIVertexShader* GetVertexShaderRHI()
        {
            if (ShaderRHI->ResourceType == ERHIResourceType::RRT_VertexShader)
                return static_cast<RHIVertexShader*>(ShaderRHI.GetReference());
            return nullptr;
        }

        RHIPixelShader* GetPixelShaderRHI()
        {
            if (ShaderRHI->ResourceType == ERHIResourceType::RRT_PixelShader)
                return static_cast<RHIPixelShader*>(ShaderRHI.GetReference());
            return nullptr;
        }

        RHIComputeShader* GetComputeShaderRHI()
        {
            if (ShaderRHI->ResourceType == ERHIResourceType::RRT_ComputeShader)
                return static_cast<RHIComputeShader*>(ShaderRHI.GetReference());
            return nullptr;
        }

        const std::string& GetName() const { return ShaderName; }

        RHIShaderRef ShaderRHI;
        EShaderStage ShaderStage;
        EShaderMetaType ShaderMetaType; // Material or Global
        std::string ShaderName;
        std::string Code;

        const std::unordered_map<uint32, RHIDescriptorSetLayoutRef>& GetDescriptorSetLayouts() const
        {
            return ShaderRHI->DescriptorSetLayouts;
        }
        RHIDescriptorSetLayout* GetDescriptorSetLayout(uint32 SetIndex) const
        {
            return ShaderRHI->DescriptorSetLayouts.at(SetIndex).GetReference();
        }

        virtual void InitRHI();

        virtual void ReleaseRHI();

    };
    using FShaderInstanceRef = std::shared_ptr<FShaderInstance>;
}