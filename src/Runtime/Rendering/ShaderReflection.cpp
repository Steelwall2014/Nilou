#include <spirv_reflect.h>
#include <shaderc/shaderc.h>
#include "ShaderReflection.h"
#include "DynamicRHI.h"
#include "RHIResources.h"
#include "ShaderType.h"

namespace nilou {

namespace shader_reflection {

bool ReflectShader(const std::string& ShaderCode, EShaderStage ShaderStage, DescriptorSetLayouts& OutLayouts, std::string& OutMessage)
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
    shaderc_compilation_status status = shaderc_result_get_compilation_status(compile_result);
    if (status != shaderc_compilation_status_success)
    {
        OutMessage = shaderc_result_get_error_message(compile_result);
        NILOU_LOG(Error, "Shader compilation error occured during shader reflection! Error message: {}", OutMessage);
        return false;
    }
    size_t codeSize = shaderc_result_get_length(compile_result);
    const char* pCode = shaderc_result_get_bytes(compile_result);

    SpvReflectShaderModule module;
    SpvReflectResult result = spvReflectCreateShaderModule(codeSize, pCode, &module);
    Ncheck(result == SPV_REFLECT_RESULT_SUCCESS);

    uint32_t ubo_count = 0;
    result = spvReflectEnumerateDescriptorBindings(&module, &ubo_count, NULL);
    Ncheck(result == SPV_REFLECT_RESULT_SUCCESS);
    auto input_ubos = std::vector<SpvReflectDescriptorBinding*>(ubo_count);
    result = spvReflectEnumerateDescriptorBindings(&module, &ubo_count, input_ubos.data());
    Ncheck(result == SPV_REFLECT_RESULT_SUCCESS);

    for (SpvReflectDescriptorBinding* input_ubo : input_ubos)
    {
        DescriptorSetLayoutBinding Binding;
        Binding.BindingIndex = input_ubo->binding;
        Binding.SetIndex = input_ubo->set;
        Binding.Name = input_ubo->name;
        if (input_ubo->decoration_flags & SPV_REFLECT_DECORATION_NON_WRITABLE)
        {
            Binding.bReadOnly = true;
        }
        if (input_ubo->decoration_flags & SPV_REFLECT_DECORATION_NON_READABLE)
        {
            Binding.bWriteOnly = true;
        }
        if (Binding.Name == "")
        {
            Binding.Name = input_ubo->block.name;
        }
        if (Binding.Name == "")
        {
            Binding.Name = input_ubo->block.type_description->type_name;
        }
        Binding.DescriptorType = static_cast<EDescriptorType>(input_ubo->descriptor_type);
        // Block
        Binding.Block.Name = input_ubo->block.name ? input_ubo->block.name : "";
        Binding.Block.Offset = input_ubo->block.offset;
        Binding.Block.AbsoluteOffset = input_ubo->block.absolute_offset;
        Binding.Block.Size = input_ubo->block.size;
        Binding.Block.PaddedSize = input_ubo->block.padded_size;
        // Block.Numeric
        // Block.Numeric.Scalar
        Binding.Block.Numeric.Scalar.Width = input_ubo->block.numeric.scalar.width;
        Binding.Block.Numeric.Scalar.Signedness = input_ubo->block.numeric.scalar.signedness;
        // Block.Numeric.Vector
        Binding.Block.Numeric.Vector.ComponentCount = input_ubo->block.numeric.vector.component_count;
        // Block.Numeric.Matrix
        Binding.Block.Numeric.Matrix.ColumnCount = input_ubo->block.numeric.matrix.column_count;
        Binding.Block.Numeric.Matrix.RowCount = input_ubo->block.numeric.matrix.row_count;
        Binding.Block.Numeric.Matrix.Stride = input_ubo->block.numeric.matrix.stride;
        // Block.Array
        Binding.Block.Array.DimsCount = input_ubo->block.array.dims_count;
        for (uint32_t i = 0; i < input_ubo->block.array.dims_count; i++)
            Binding.Block.Array.Dims[i] = input_ubo->block.array.dims[i];
        // Image
        Binding.Image.Dim = static_cast<EImageDim>(input_ubo->image.dim);
        Binding.Image.Depth = input_ubo->image.depth;
        Binding.Image.Arrayed = input_ubo->image.arrayed;
        Binding.Image.MultiSampled = input_ubo->image.ms;
        Binding.Image.Sampled = input_ubo->image.sampled;
        Binding.Image.Format = static_cast<EImageFormat>(input_ubo->image.image_format);
        Binding.Image.Arrayed = input_ubo->image.arrayed;
        // Array
        Binding.Array.DimsCount = input_ubo->array.dims_count;
        for (uint32_t i = 0; i < input_ubo->array.dims_count; i++)
            Binding.Array.Dims[i] = input_ubo->array.dims[i];
        OutLayouts[Binding.SetIndex][Binding.BindingIndex] = Binding;
    }

    spvReflectDestroyShaderModule(&module);
    shaderc_compiler_release(shader_compiler);

    return true;
}

} // namespace shader_reflection

bool ReflectShader(const std::string& ShaderCode, EShaderStage ShaderStage, std::unordered_map<uint32, TRefCountPtr<class RHIDescriptorSetLayout>>& OutLayouts, std::optional<RHIPushConstantRange>& OutPushConstantRange, std::string& OutMessage)
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
    
    bool bSuccess = ReflectShader(compile_result, OutLayouts, OutPushConstantRange, OutMessage);
    shaderc_result_release(compile_result);
    shaderc_compiler_release(shader_compiler);

    return bSuccess;
}

bool ReflectShader(shaderc_compilation_result_t compile_result, std::unordered_map<uint32, TRefCountPtr<class RHIDescriptorSetLayout>>& OutLayouts, std::optional<RHIPushConstantRange>& OutPushConstantRange, std::string& OutMessage)
{
    size_t codeSize = shaderc_result_get_length(compile_result);
    const char* pCode = shaderc_result_get_bytes(compile_result);

    SpvReflectShaderModule module;
    if (spvReflectCreateShaderModule(codeSize, pCode, &module) != SPV_REFLECT_RESULT_SUCCESS)
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

} // namespace nilou