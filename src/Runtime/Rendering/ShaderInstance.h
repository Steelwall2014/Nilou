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
            EPipelineStage InPipelineStage,
            EShaderMetaType InShaderMetaType)
            : ShaderName(InShaderName)
            , Code(InCode)
            , PipelineStage(InPipelineStage)
            , ShaderMetaType(InShaderMetaType)
        { }

        RHIVertexShader* GetVertexShaderRHI()
        {
            if (ShaderRHI->ResourceType == ERHIResourceType::RRT_VertexShader)
                return static_cast<RHIVertexShader*>(ShaderRHI.get());
            return nullptr;
        }

        RHIPixelShader* GetPixelShaderRHI()
        {
            if (ShaderRHI->ResourceType == ERHIResourceType::RRT_PixelShader)
                return static_cast<RHIPixelShader*>(ShaderRHI.get());
            return nullptr;
        }

        RHIComputeShader* GetComputeShaderRHI()
        {
            if (ShaderRHI->ResourceType == ERHIResourceType::RRT_ComputeShader)
                return static_cast<RHIComputeShader*>(ShaderRHI.get());
            return nullptr;
        }

        RHIShaderRef ShaderRHI;
        EPipelineStage PipelineStage;
        EShaderMetaType ShaderMetaType; // Material or Global
        std::string ShaderName;
        std::string Code;
        shader_reflection::DescriptorSetLayouts DescriptorSetLayouts;

        virtual void InitRHI();

        virtual void ReleaseRHI();

    };
    using FShaderInstanceRef = std::shared_ptr<FShaderInstance>;
}