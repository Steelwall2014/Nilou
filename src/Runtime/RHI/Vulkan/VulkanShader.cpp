#include "VulkanShader.h"
#include "VulkanDynamicRHI.h"

namespace nilou {


RHIVertexShaderRef FVulkanDynamicRHI::RHICreateVertexShader(const std::string& code)
{
    auto [Module, result] = 
        RHICompileShaderInternal(code, shaderc_vertex_shader);

    if (Module && result)
    {
        VulkanVertexShaderRef VulkanShader = std::make_shared<VulkanVertexShader>();
        VulkanShader->Module = Module;
        VulkanShader->ShadercResult = result;
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
        VulkanPixelShaderRef VulkanShader = std::make_shared<VulkanPixelShader>();
        VulkanShader->Module = Module;
        VulkanShader->ShadercResult = result;
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
        VulkanComputeShaderRef VulkanShader = std::make_shared<VulkanComputeShader>();
        VulkanShader->Module = Module;
        VulkanShader->ShadercResult = result;
        return VulkanShader;
    }

    NILOU_LOG(Error, "failed to create compute shader!");
    return nullptr;
}

void FVulkanDynamicRHI::RHIDestroyShader(RHIShader* Shader)
{
    VkShaderModule Module = VK_NULL_HANDLE;
    shaderc_compilation_result_t Result = nullptr;
    if (Shader->ResourceType == ERHIResourceType::RRT_VertexShader)
    {
        VulkanVertexShader* VulkanShader = reinterpret_cast<VulkanVertexShader*>(Shader);
        Module = VulkanShader->Module;
        Result = VulkanShader->ShadercResult;
    }
    else if (Shader->ResourceType == ERHIResourceType::RRT_PixelShader)
    {
        VulkanPixelShader* VulkanShader = reinterpret_cast<VulkanPixelShader*>(Shader);
        Module = VulkanShader->Module;
        Result = VulkanShader->ShadercResult;
    }
    else if (Shader->ResourceType == ERHIResourceType::RRT_ComputeShader)
    {
        VulkanComputeShader* VulkanShader = reinterpret_cast<VulkanComputeShader*>(Shader);
        Module = VulkanShader->Module;
        Result = VulkanShader->ShadercResult;
    }
    if (Module != VK_NULL_HANDLE)
        vkDestroyShaderModule(device, Module, nullptr);
    if (Result != nullptr)
        shaderc_result_release(Result);
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
        return {};
    }

    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = shaderc_result_get_length(result);
    createInfo.pCode = reinterpret_cast<const uint32*>(shaderc_result_get_bytes(result));
    VkShaderModule Module{};
    if (vkCreateShaderModule(device, &createInfo, nullptr, &Module) != VK_SUCCESS) {
        return {};
    }

    return { Module, result };

}

}