#include <spirv_reflect.h>
#include <shaderc/shaderc.h>
#include "Vulkan/VulkanDynamicRHI.h"
#include "OpenGL/OpenGLDynamicRHI.h"
#include "UniformBuffer.h"
#include "PipelineStateCache.h"

namespace nilou {
	RHITextureParams RHITextureParams::DefaultParams = RHITextureParams();

	FDynamicRHI *FDynamicRHI::DynamicRHI = nullptr;

	FDynamicRHI *FDynamicRHI::GetDynamicRHI()
	{
		return FDynamicRHI::DynamicRHI;
	}

	ivec3 FDynamicRHI::RHIGetSparseTexturePageSize(ETextureDimension TextureType, EPixelFormat PixelFormat) 
	{ 
		return FDynamicRHI::SparseTextureTileSizes[(int)TextureType][(int)PixelFormat]; 
	}

	void FDynamicRHI::CreateDynamicRHI_RenderThread(const GfxConfiguration& configs)
	{
		if (!strcmp(configs.defaultRHI, "opengl"))
		{
			// FDynamicRHI::DynamicRHI = new FOpenGLDynamicRHI(configs);
		}
		else if (!strcmp(configs.defaultRHI, "vulkan"))
		{
			FDynamicRHI::DynamicRHI = new FVulkanDynamicRHI(configs);
		}
	}

    int FDynamicRHI::Initialize()
    {
        return 1;
    }

	void FDynamicRHI::Finalize()
	{
		FPipelineStateCache::ClearCacheGraphicsPSO();
	}

    static EDescriptorType TranslateDescriptorType(SpvReflectDescriptorType InputType)
    {
        switch (InputType)
        {
        case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER:
            return EDescriptorType::Sampler;
        case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE:
            return EDescriptorType::StorageImage;
        case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
            return EDescriptorType::UniformBuffer;
        case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER:
            return EDescriptorType::StorageBuffer;
        default:
            return EDescriptorType::Num;
        }
    }

    void FDynamicRHI::ReflectShader(RHIDescriptorSetsLayout& DescriptorSetsLayout, shaderc_compilation_result_t compile_result)
    {
        shaderc_compilation_status status = shaderc_result_get_compilation_status(compile_result);
        assert(status == shaderc_compilation_status_success);
        size_t codeSize = shaderc_result_get_length(compile_result);
        const char* pCode = shaderc_result_get_bytes(compile_result);

        SpvReflectShaderModule module;
        SpvReflectResult result = spvReflectCreateShaderModule(codeSize, pCode, &module);
        assert(result == SPV_REFLECT_RESULT_SUCCESS);

        uint32_t ubo_count = 0;
        result = spvReflectEnumerateDescriptorBindings(&module, &ubo_count, NULL);
        assert(result == SPV_REFLECT_RESULT_SUCCESS);
        std::vector<SpvReflectDescriptorBinding*> input_ubos = std::vector<SpvReflectDescriptorBinding*>(ubo_count);
        result = spvReflectEnumerateDescriptorBindings(&module, &ubo_count, input_ubos.data());
        assert(result == SPV_REFLECT_RESULT_SUCCESS);

        for (auto input_ubo : input_ubos)
        {
            RHIDescriptorSetLayoutBinding Binding;
            Binding.Name = input_ubo->name;
            Binding.BindingIndex = input_ubo->binding;
            Binding.SetIndex = input_ubo->set;
            Binding.DescriptorType = TranslateDescriptorType(input_ubo->descriptor_type);
            DescriptorSetsLayout[input_ubo->set][input_ubo->name] = Binding;
        }

        spvReflectDestroyShaderModule(&module);
    }

	ivec3 FDynamicRHI::SparseTextureTileSizes[(int)ETextureDimension::TextureDimensionsNum][(int)EPixelFormat::PF_MAX];
}