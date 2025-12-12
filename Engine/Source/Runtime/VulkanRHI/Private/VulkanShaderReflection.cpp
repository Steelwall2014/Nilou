#include <spirv_reflect.h>
#include <shaderc/shaderc.h>
#include "VulkanDynamicRHI.h"
#include "RHIResources.h"

namespace nilou {

bool FVulkanDynamicRHI::RHIReflectShaderInternal(TArrayView<uint8> ByteCode, std::unordered_map<uint32, TRefCountPtr<class RHIDescriptorSetLayout>>& OutLayouts, std::optional<RHIPushConstantRange>& OutPushConstantRange, std::string& OutMessage)
{
    SpvReflectShaderModule module;
    if (spvReflectCreateShaderModule(ByteCode.Num(), ByteCode.GetData(), &module) != SPV_REFLECT_RESULT_SUCCESS)
    {
        OutMessage = "Failed to create shader module!";
        return false;
    }

    uint32_t ubo_count = 0;
    if (spvReflectEnumerateDescriptorBindings(&module, &ubo_count, NULL) != SPV_REFLECT_RESULT_SUCCESS)
    {
        OutMessage = "Failed to enumerate descriptor bindings!";
        return false;
    }
    auto input_ubos = std::vector<SpvReflectDescriptorBinding*>(ubo_count);
    if (spvReflectEnumerateDescriptorBindings(&module, &ubo_count, input_ubos.data()) != SPV_REFLECT_RESULT_SUCCESS)
    {
        OutMessage = "Failed to enumerate descriptor bindings!";
        return false;
    }

    std::unordered_map<uint32, std::vector<RHIDescriptorSetLayoutBinding>> BindingsPerSet;
    for (SpvReflectDescriptorBinding* input_ubo : input_ubos)
    {
        RHIDescriptorSetLayoutBinding Binding;
        Binding.BindingIndex = input_ubo->binding;
        Binding.DescriptorType = static_cast<EDescriptorType>(input_ubo->descriptor_type);
        Binding.DescriptorCount = 1;
        Binding.Name = input_ubo->name;
        if (Binding.Name == "")
        {
            Binding.Name = input_ubo->block.name;
        }
        if (Binding.Name == "")
        {
            Binding.Name = input_ubo->block.type_description->type_name;
        }
        Binding.BlockSize = input_ubo->block.padded_size;
        for (uint32_t i = 0; i < input_ubo->block.member_count; i++)
        {
            SpvReflectBlockVariable& Member = input_ubo->block.members[i];
            Binding.Members.push_back({Member.name, Member.offset});
        }
        if (input_ubo->decoration_flags & SPV_REFLECT_DECORATION_NON_WRITABLE)
            Binding.Flags |= EDescriptorDecorationFlags::NonWritable;
        if (input_ubo->decoration_flags & SPV_REFLECT_DECORATION_NON_READABLE)
            Binding.Flags |= EDescriptorDecorationFlags::NonReadable;
        BindingsPerSet[input_ubo->set].push_back(Binding);
    }
    
    uint32_t push_constant_count = 0;
    if (spvReflectEnumeratePushConstantBlocks(&module, &push_constant_count, NULL) != SPV_REFLECT_RESULT_SUCCESS)
    {
        OutMessage = "Failed to enumerate push constant blocks!";
        return false;
    }
    if (push_constant_count > 0)
    {
        OutPushConstantRange = RHIPushConstantRange();
        auto input_push_constants = std::vector<SpvReflectBlockVariable*>(push_constant_count);
        if (spvReflectEnumeratePushConstantBlocks(&module, &push_constant_count, input_push_constants.data()) != SPV_REFLECT_RESULT_SUCCESS)
        {
            OutMessage = "Failed to enumerate push constant blocks!";
            return false;
        }
        for (SpvReflectBlockVariable* input_push_constant : input_push_constants)
        {
            OutPushConstantRange->Size = input_push_constant->padded_size;
            for (uint32_t i = 0; i < input_push_constant->member_count; i++)
            {
                SpvReflectBlockVariable& Member = input_push_constant->members[i];
                OutPushConstantRange->Members.push_back({Member.name, Member.offset, Member.size});
            }
        }
    }

    spvReflectDestroyShaderModule(&module);

    for (auto& [SetIndex, Bindings] : BindingsPerSet)
    {
        OutLayouts[SetIndex] = RHICreateDescriptorSetLayout(Bindings);
        if (OutLayouts[SetIndex] == nullptr)
        {
            OutMessage = "Failed to create descriptor set layout!";
            OutLayouts.clear();
            return false;
        }
    }

    return true;
}

bool FVulkanDynamicRHI::RHIReflectShader(
    const std::string& ShaderCode, 
    EShaderStage ShaderStage, 
    std::unordered_map<uint32, TRefCountPtr<RHIDescriptorSetLayout>>& OutLayouts, 
    std::optional<RHIPushConstantRange>& OutPushConstantRange, 
    std::string& OutMessage)
{
    shaderc_compiler_t shader_compiler = shaderc_compiler_initialize();
    shaderc_shader_kind shader_kind;
    switch (ShaderStage) 
    {
    case EShaderStage::Vertex:
        shader_kind = shaderc_glsl_vertex_shader;
        break;
    case EShaderStage::Pixel:
        shader_kind = shaderc_glsl_fragment_shader;
        break;
    case EShaderStage::Compute:
        shader_kind = shaderc_glsl_compute_shader;
        break;
    default:
        Ncheck(0);
    };
    shaderc_compilation_result_t compile_result = shaderc_compile_into_spv(shader_compiler, 
        ShaderCode.c_str(), ShaderCode.size(), shader_kind, 
        "", "main", nullptr);
    if (shaderc_result_get_compilation_status(compile_result) != shaderc_compilation_status_success)
    {
        OutMessage = shaderc_result_get_error_message(compile_result);
        return false;
    }
    
    TArrayView<uint8> ByteCode = TArrayView<uint8>((uint8*)shaderc_result_get_bytes(compile_result), shaderc_result_get_length(compile_result));
    bool bSuccess = RHIReflectShaderInternal(ByteCode, OutLayouts, OutPushConstantRange, OutMessage);
    shaderc_result_release(compile_result);
    shaderc_compiler_release(shader_compiler);

    return bSuccess;
}

} // namespace nilou