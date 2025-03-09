#include "VulkanDevice.h"
#include "VulkanShader.h"
#include "VulkanDynamicRHI.h"
#include "Common/Log.h"

namespace nilou {


RHIVertexShaderRef FVulkanDynamicRHI::RHICreateVertexShader(const std::string& code)
{
    auto [Module, result] = 
        RHICompileShaderInternal(code, shaderc_vertex_shader);

    if (Module && result)
    {
        VulkanVertexShaderRef VulkanShader = new VulkanVertexShader(Device->Handle);
        VulkanShader->Module = Module;
        return VulkanShader;
    }

    NILOU_LOG(Error, "failed to create vertex shader!");
    return nullptr;
}

RHIPixelShaderRef FVulkanDynamicRHI::RHICreatePixelShader(const std::string& code)
{
    auto [Module, result] = 
        RHICompileShaderInternal(code, shaderc_fragment_shader);

    if (Module && result)
    {
        VulkanPixelShaderRef VulkanShader = new VulkanPixelShader(Device->Handle);
        VulkanShader->Module = Module;
        return VulkanShader;
    }

    NILOU_LOG(Error, "failed to create pixel shader!");
    return nullptr;
}

RHIComputeShaderRef FVulkanDynamicRHI::RHICreateComputeShader(const std::string& code)
{
    auto [Module, result] = 
        RHICompileShaderInternal(code, shaderc_compute_shader);

    if (Module && result)
    {
        VulkanComputeShaderRef VulkanShader = new VulkanComputeShader(Device->Handle);
        VulkanShader->Module = Module;
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
        NILOU_LOG(Error, "Shader compilation error! Error message: {}. The code is written to {}", msg, "ShaderCompilationErrors.txt");
        std::ofstream out{"ShaderCompilationErrors.txt", std::ios::app};
        out << msg << "\n\n" << code << std::endl;
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