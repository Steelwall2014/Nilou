#include "VulkanDevice.h"
#include "VulkanShader.h"
#include "VulkanDynamicRHI.h"
#include "Logging/LogMacros.h"

namespace nilou {


RHIVertexShaderRef FVulkanDynamicRHI::RHICreateVertexShader(const std::string& code, const std::string& DebugName)
{
    auto [Module, result] = 
        RHICompileShaderInternal(code, shaderc_vertex_shader, DebugName);

    if (Module && result)
    {
        VulkanVertexShaderRef VulkanShader = TRefCountPtr(new VulkanVertexShader(Device->Handle, DebugName));
        VulkanShader->Module = Module;
        std::string OutMessage;
        TArrayView<uint8> ByteCode = TArrayView<uint8>((uint8*)shaderc_result_get_bytes(result), shaderc_result_get_length(result));
        // bool bSuccess = RHIReflectShaderInternal(ByteCode, VulkanShader->DescriptorSetLayouts, VulkanShader->PushConstantRange, OutMessage);
        // if (!bSuccess)
        // {
        //     NILOU_LOG(Error, "failed to reflect vertex shader! {}, Dump the code to log", OutMessage);
        //     NILOU_LOG(Error, "\n{}", code);
        //     Ncheck(false);
        //     return nullptr;
        // }
        return VulkanShader;
    }

    NILOU_LOG(Error, "failed to create vertex shader!");
    return nullptr;
}

RHIPixelShaderRef FVulkanDynamicRHI::RHICreatePixelShader(const std::string& code, const std::string& DebugName)
{
    auto [Module, result] = 
        RHICompileShaderInternal(code, shaderc_fragment_shader, DebugName);

    if (Module && result)
    {
        VulkanPixelShaderRef VulkanShader = TRefCountPtr(new VulkanPixelShader(Device->Handle, DebugName));
        VulkanShader->Module = Module;
        std::string OutMessage;
        TArrayView<uint8> ByteCode = TArrayView<uint8>((uint8*)shaderc_result_get_bytes(result), shaderc_result_get_length(result));
        // bool bSuccess = RHIReflectShaderInternal(ByteCode, VulkanShader->DescriptorSetLayouts, VulkanShader->PushConstantRange, OutMessage);
        // if (!bSuccess)
        // {
        //     NILOU_LOG(Error, "failed to reflect pixel shader! {}, Dump the code to log", OutMessage);
        //     NILOU_LOG(Error, "\n{}", code);
        //     Ncheck(false);
        //     return nullptr;
        // }
        return VulkanShader;
    }

    NILOU_LOG(Error, "failed to create pixel shader!");
    return nullptr;
}

RHIComputeShaderRef FVulkanDynamicRHI::RHICreateComputeShader(const std::string& code, const std::string& DebugName)
{
    auto [Module, result] = 
        RHICompileShaderInternal(code, shaderc_compute_shader, DebugName);

    if (Module && result)
    {
        VulkanComputeShaderRef VulkanShader = TRefCountPtr(new VulkanComputeShader(Device->Handle, DebugName));
        VulkanShader->Module = Module;
        std::string OutMessage;
        TArrayView<uint8> ByteCode = TArrayView<uint8>((uint8*)shaderc_result_get_bytes(result), shaderc_result_get_length(result));
        // bool bSuccess = RHIReflectShaderInternal(ByteCode, VulkanShader->DescriptorSetLayouts, VulkanShader->PushConstantRange, OutMessage);
        // if (!bSuccess)
        // {
        //     NILOU_LOG(Error, "failed to reflect compute shader! {}, Dump the code to log", OutMessage);
        //     NILOU_LOG(Error, "\n{}", code);
        //     Ncheck(false);
        //     return nullptr;
        // }
        return VulkanShader;
    }

    NILOU_LOG(Error, "failed to create compute shader!");
    return nullptr;
}

template<typename TShader>
TRefCountPtr<TShader> FVulkanDynamicRHI::RHICreateShaderInternal(TArrayView<uint8> ByteCode, const std::string& DebugName)
{
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = ByteCode.Num();
    createInfo.pCode = reinterpret_cast<const uint32*>(ByteCode.GetData());
    VkShaderModule Module{};
    VK_CHECK_RESULT(vkCreateShaderModule(Device->Handle, &createInfo, nullptr, &Module));
#if VULKAN_ENABLE_DRAW_MARKERS
    Device->SetDebugUtilsObjectName(VK_OBJECT_TYPE_SHADER_MODULE, (uint64_t)Module, DebugName.c_str());
#endif
    TRefCountPtr<TShader> Shader = TRefCountPtr(new TShader(Device->Handle, DebugName));
    Shader->Module = Module;
    std::string OutMessage;
    // bool bSuccess = RHIReflectShaderInternal(ByteCode, Shader->DescriptorSetLayouts, Shader->PushConstantRange, OutMessage);
    // if (!bSuccess)
    // {
    //     NILOU_LOG(Fatal, "failed to reflect shader! {}", OutMessage);
    //     return nullptr;
    // }
    return Shader;
}

RHIVertexShaderRef FVulkanDynamicRHI::RHICreateVertexShader(TArrayView<uint8> ByteCode, const std::string& DebugName)
{
    return RHICreateShaderInternal<VulkanVertexShader>(ByteCode, DebugName);
}

RHIPixelShaderRef FVulkanDynamicRHI::RHICreatePixelShader(TArrayView<uint8> ByteCode, const std::string& DebugName)
{
    return RHICreateShaderInternal<VulkanPixelShader>(ByteCode, DebugName);
}
RHIComputeShaderRef FVulkanDynamicRHI::RHICreateComputeShader(TArrayView<uint8> ByteCode, const std::string& DebugName)
{
    return RHICreateShaderInternal<VulkanComputeShader>(ByteCode, DebugName);
}

std::pair<VkShaderModule, shaderc_compilation_result_t> 
FVulkanDynamicRHI::RHICompileShaderInternal(const std::string& code, shaderc_shader_kind shader_kind, const std::string& DebugName)
{
    shaderc_compile_options_t options = shaderc_compile_options_initialize();
    shaderc_compile_options_set_optimization_level(options, shaderc_optimization_level_zero);
    shaderc_compile_options_set_generate_debug_info(options);
    // shaderc_compile_options_set_auto_bind_uniforms(options, true);
    shaderc_compilation_result_t result = shaderc_compile_into_spv(shader_compiler, 
        code.c_str(), code.size(), shader_kind, 
        "", "main", options);
    shaderc_compile_options_release(options);
    shaderc_compilation_status status = shaderc_result_get_compilation_status(result);
    if (status != shaderc_compilation_status_success) {
        const char* msg = shaderc_result_get_error_message(result);
        NILOU_LOG(Error, "Shader compilation error! Error message: {}", msg);
        NILOU_LOG(Error, "Dump the code to log\n{}", code);
        Ncheck(false);
        return {};
    }

    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = shaderc_result_get_length(result);
    createInfo.pCode = reinterpret_cast<const uint32*>(shaderc_result_get_bytes(result));
    VkShaderModule Module{};
    VK_CHECK_RESULT(vkCreateShaderModule(Device->Handle, &createInfo, nullptr, &Module));
#if VULKAN_ENABLE_DRAW_MARKERS
    Device->SetDebugUtilsObjectName(VK_OBJECT_TYPE_SHADER_MODULE, (uint64_t)Module, DebugName.c_str());
#endif
    return { Module, result };

}

}