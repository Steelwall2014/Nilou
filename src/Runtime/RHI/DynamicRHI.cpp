#include <glslang/Public/ResourceLimits.h>
#include <glslang/MachineIndependent/localintermediate.h>
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
			FDynamicRHI::DynamicRHI = new FOpenGLDynamicRHI(configs);
		}
		else if (!strcmp(configs.defaultRHI, "vulkan"))
		{
			FDynamicRHI::DynamicRHI = new FVulkanDynamicRHI(configs);
		}
	}

    int FDynamicRHI::Initialize()
    {
        glslang::InitializeProcess();
        *GetResources() = *GetDefaultResources();
        GetResources()->maxAtomicCounterBindings = 5;
        return 1;
    }

	void FDynamicRHI::Finalize()
	{
		FPipelineStateCache::ClearCacheGraphicsPSO();
	}

    static EShaderParameterType TranslateToShaderParameterType(const glslang::TType *Type)
    {
        EShaderParameterType type = EShaderParameterType::SPT_None;
        glslang::TBasicType BasicType = Type->getBasicType();
        switch (BasicType)
        {
        case glslang::TBasicType::EbtBlock:
            type = EShaderParameterType::SPT_UniformBuffer;
            break;
        case glslang::TBasicType::EbtSampler:
            type = EShaderParameterType::SPT_Sampler;
            break;
        // case glslang::TBasicType::EbtAtomicUint:
        //     type = EShaderParameterType::SPT_AtomicUint;
        //     break;
        // case glslang::TBasicType::EbtUint:
        //     type = EShaderParameterType::SPT_Uint;
        //     break;
        // case glslang::TBasicType::EbtFloat:
        //     type = EShaderParameterType::SPT_Float;
        //     break;
        // case glslang::TBasicType::EbtInt:
        //     type = EShaderParameterType::SPT_Int;
        //     break;
        }  
        if (Type->isImage())
            type = EShaderParameterType::SPT_Image;
        
        return type;
    }

    void FDynamicRHI::AllocateParameterBindingPoint(FRHIPipelineLayout* PipelineLayout, const FGraphicsPipelineStateInitializer &Initializer)
    {
        glslang::TProgram ProgramGlsl;
        bool link_res = false;
        if (Initializer.ComputeShader != nullptr)
        {
            ProgramGlsl.addShader(Initializer.ComputeShader->ShaderGlsl);
            link_res = ProgramGlsl.link(EShMsgDefault);
            if (!link_res)
            {
                std::string info = ProgramGlsl.getInfoLog();
                std::string debuginfo = ProgramGlsl.getInfoDebugLog();
                NILOU_LOG(Error, "{}\n{}", info, debuginfo);
				return;
            }
            ProgramGlsl.buildReflection();
        }
        else if (
            Initializer.VertexShader != nullptr && 
            Initializer.PixelShader != nullptr)
        {
            RHIGetError();
            ProgramGlsl.addShader(Initializer.VertexShader->ShaderGlsl);
            ProgramGlsl.addShader(Initializer.PixelShader->ShaderGlsl);
            link_res = ProgramGlsl.link(EShMsgDefault);
            if (!link_res)
            {
                std::string info = ProgramGlsl.getInfoLog();
                std::string debuginfo = ProgramGlsl.getInfoDebugLog();
                NILOU_LOG(Error, "{}\n{}", info, debuginfo);
				return;
            }
            ProgramGlsl.buildReflection();
        }

        FRHIDescriptorSet &VertexDescriptorSet = PipelineLayout->DescriptorSets[EPipelineStage::PS_Vertex];
        FRHIDescriptorSet &PixelDescriptorSet = PipelineLayout->DescriptorSets[EPipelineStage::PS_Pixel];
        FRHIDescriptorSet &ComputeDescriptorSet = PipelineLayout->DescriptorSets[EPipelineStage::PS_Compute];

        
        int Buffer_block_num = ProgramGlsl.getNumBufferBlocks();

            
        int max_sampler_binding_point = -1;
        int NumUniformVariable = ProgramGlsl.getNumUniformVariables();
        for (int i = 0; i < NumUniformVariable; i++)
        {
            const glslang::TObjectReflection &refl = ProgramGlsl.getUniform(i);
            FRHIDescriptorSetLayoutBinding binding;
            binding.Name = refl.name;
            binding.ParameterType = TranslateToShaderParameterType(refl.getType());
            binding.BindingPoint = refl.getBinding();
			if (binding.BindingPoint == -1)
				continue;
            // if (binding.ParameterType == EShaderParameterType::SPT_Image)
            // {
            //     if (binding.BindingPoint == -1)
            //     {
            //         NILOU_LOG(Error, "image1/2/3D variables must have an explicit binding point");
            //         continue;
            //     }
            // }
            // else if (binding.ParameterType == EShaderParameterType::SPT_Sampler)
            // {
            //     binding.BindingPoint = glGetUniformLocation(PipelineResource, binding.Name.c_str());
            //     if (binding.BindingPoint == -1)
            //     {
            //         NILOU_LOG(Warning, "Shader parameter {} is omitted in glsl", binding.Name)
            //         continue;
            //     }
            //     max_sampler_binding_point = std::max(max_sampler_binding_point, binding.BindingPoint);
            // }
            // else if (binding.ParameterType == EShaderParameterType::SPT_AtomicUint)
            // {
            //     if (binding.BindingPoint == -1)
            //     {
            //         NILOU_LOG(Error, "Atomic uint variables must have an explicit binding point");
            //         continue;
            //     }
            // }
            // else if (binding.ParameterType == EShaderParameterType::SPT_Float || 
            //          binding.ParameterType == EShaderParameterType::SPT_Int || 
            //          binding.ParameterType == EShaderParameterType::SPT_Uint)
            // {
            //     binding.BindingPoint = glGetUniformLocation(PipelineResource, binding.Name.c_str());
            //     if (binding.BindingPoint == -1)
            //     {
            //         continue;
            //     }
            // }
            // else 
            // {
            //     continue;
            // }

            if (refl.stages & EShLangVertexMask)
            {
                VertexDescriptorSet.Bindings[binding.Name] = binding;
            }
            if (refl.stages & EShLangFragmentMask)
            {
                PixelDescriptorSet.Bindings[binding.Name] = binding;
            }
            if (refl.stages & EShLangComputeMask)
            {
                ComputeDescriptorSet.Bindings[binding.Name] = binding;
            }
        }

        int NumUniformBlock = ProgramGlsl.getNumUniformBlocks();
        for (int i = 0; i < NumUniformBlock; i++)
        {
            const glslang::TObjectReflection &refl = ProgramGlsl.getUniformBlock(i);
            FRHIDescriptorSetLayoutBinding binding;
            binding.Name = refl.name;
            binding.ParameterType = TranslateToShaderParameterType(refl.getType());
            binding.BindingPoint = refl.getBinding();
			if (binding.BindingPoint == -1)
				continue;
            // if (binding.BindingPoint == -1)
            // {
            //     if (binding.ParameterType == EShaderParameterType::SPT_UniformBuffer)
            //     {
            //         int block_index = glGetUniformBlockIndex(PipelineResource, binding.Name.c_str());
            //         if (block_index == -1)
            //         {
            //             NILOU_LOG(Warning, "Shader parameter {} is omitted in glsl", binding.Name)
            //             continue;
            //         }
            //         glUniformBlockBinding(PipelineResource, block_index, ++max_sampler_binding_point);
            //         binding.BindingPoint = max_sampler_binding_point;
            //     }
            //     else if (binding.ParameterType == EShaderParameterType::SPT_AtomicUint)
            //     {
            //         NILOU_LOG(Error, "Atomic uint variables must have an explicit binding point");
            //         continue;
            //     }
            // }

            if (refl.stages & EShLangVertexMask)
            {
                VertexDescriptorSet.Bindings[binding.Name] = binding;
            }
            if (refl.stages & EShLangFragmentMask)
            {
                PixelDescriptorSet.Bindings[binding.Name] = binding;
            }
            if (refl.stages & EShLangComputeMask)
            {
                ComputeDescriptorSet.Bindings[binding.Name] = binding;
            }
        }
        
    }

	ivec3 FDynamicRHI::SparseTextureTileSizes[(int)ETextureDimension::TextureDimensionsNum][(int)EPixelFormat::PF_PixelFormatNum];
}