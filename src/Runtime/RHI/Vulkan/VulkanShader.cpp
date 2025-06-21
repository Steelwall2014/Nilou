#include "VulkanDevice.h"
#include "VulkanShader.h"
#include "VulkanDynamicRHI.h"
#include "Common/Log.h"
#include "ShaderReflection.h"

namespace nilou {


RHIVertexShaderRef FVulkanDynamicRHI::RHICreateVertexShader(const std::string& code, const std::string& DebugName)
{
    auto [Module, result] = 
        RHICompileShaderInternal(code, shaderc_vertex_shader);

    if (Module && result)
    {
        VulkanVertexShaderRef VulkanShader = new VulkanVertexShader(Device->Handle);
        VulkanShader->DebugName = DebugName;
        VulkanShader->Module = Module;
        std::string OutMessage;
        bool bSuccess = ReflectShader(result, VulkanShader->DescriptorSetLayouts, OutMessage);
        if (!bSuccess)
        {
            NILOU_LOG(Error, "failed to reflect vertex shader! {}, Dump the code to log", OutMessage);
            NILOU_LOG(Error, "\n{}", code);
            return nullptr;
        }
        return VulkanShader;
    }

    NILOU_LOG(Error, "failed to create vertex shader!");
    return nullptr;
}

RHIPixelShaderRef FVulkanDynamicRHI::RHICreatePixelShader(const std::string& code, const std::string& DebugName)
{
    auto [Module, result] = 
        RHICompileShaderInternal(code, shaderc_fragment_shader);

    if (Module && result)
    {
        VulkanPixelShaderRef VulkanShader = new VulkanPixelShader(Device->Handle);
        VulkanShader->DebugName = DebugName;
        VulkanShader->Module = Module;
        std::string OutMessage;
        bool bSuccess = ReflectShader(result, VulkanShader->DescriptorSetLayouts, OutMessage);
        if (!bSuccess)
        {
            NILOU_LOG(Error, "failed to reflect pixel shader! {}, Dump the code to log", OutMessage);
            NILOU_LOG(Error, "\n{}", code);
            return nullptr;
        }
        return VulkanShader;
    }

    NILOU_LOG(Error, "failed to create pixel shader!");
    return nullptr;
}

RHIComputeShaderRef FVulkanDynamicRHI::RHICreateComputeShader(const std::string& code, const std::string& DebugName)
{
    auto [Module, result] = 
        RHICompileShaderInternal(code, shaderc_compute_shader);

    if (Module && result)
    {
        VulkanComputeShaderRef VulkanShader = new VulkanComputeShader(Device->Handle);
        VulkanShader->DebugName = DebugName;
        VulkanShader->Module = Module;
        std::string OutMessage;
        bool bSuccess = ReflectShader(result, VulkanShader->DescriptorSetLayouts, OutMessage);
        if (!bSuccess)
        {
            NILOU_LOG(Error, "failed to reflect compute shader! {}, Dump the code to log", OutMessage);
            NILOU_LOG(Error, "\n{}", code);
            return nullptr;
        }
        return VulkanShader;
    }

    NILOU_LOG(Error, "failed to create compute shader!");
    return nullptr;
}

std::pair<VkShaderModule, shaderc_compilation_result_t> 
FVulkanDynamicRHI::RHICompileShaderInternal(const std::string& code, shaderc_shader_kind shader_kind)
{
    shaderc_compilation_result_t result = shaderc_compile_into_spv(shader_compiler, 
        code.c_str(), code.size(), shader_kind, 
        "", "main", nullptr);
    shaderc_compilation_status status = shaderc_result_get_compilation_status(result);
    if (status != shaderc_compilation_status_success) {
        const char* msg = shaderc_result_get_error_message(result);
        NILOU_LOG(Error, "Shader compilation error! Error message: {}, Dump the code to log", msg);
        NILOU_LOG(Error, "\n{}", code);
        NILOU_LOG(Fatal, "");
        return {};
    }

    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = shaderc_result_get_length(result);
    createInfo.pCode = reinterpret_cast<const uint32*>(shaderc_result_get_bytes(result));
    VkShaderModule Module{};
    VK_CHECK_RESULT(vkCreateShaderModule(Device->Handle, &createInfo, nullptr, &Module));

    return { Module, result };

}

}