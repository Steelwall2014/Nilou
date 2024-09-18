#include "GenerateMips.h"
#include "Shader.h"
#include "ShaderInstance.h"
#include "UniformBuffer.h"
#include "RHICommandList.h"

namespace nilou {

DECLARE_GLOBAL_SHADER(FGenerateMipsCS)
IMPLEMENT_SHADER_TYPE(FGenerateMipsCS, "/Shaders/GlobalShaders/ComputeGenerateMips.glsl", EShaderFrequency::SF_Compute, Global);

BEGIN_UNIFORM_BUFFER_STRUCT(FGenerateMipsCB)
    SHADER_PARAMETER(vec2, TexelSize)
END_UNIFORM_BUFFER_STRUCT()

constexpr int GroupSize = 8;

void FGenerateMips::Execute(RenderGraph& Graph, RDGTexture* Texture, RHISamplerState* Sampler)
{
    Ncheck(Texture);
    Ncheck(Sampler);

    const RDGTextureDesc& Desc = Texture->Desc;

    FShaderInstance* Shader = GetGlobalShader<FGenerateMipsCS>();
    FRHIPipelineState* PSO = RHICreateComputePipelineState(Shader->GetComputeShaderRHI());

    for (int MipLevel = 1; MipLevel < Desc.NumMips; MipLevel++)
    {
        int TextureSizeX = std::max(Desc.SizeX >> MipLevel, 1u);
        int TextureSizeY = std::max(Desc.SizeY >> MipLevel, 1u);

        RDGBuffer* GenerateMipsCB = Graph.CreateUniformBuffer<FGenerateMipsCB>(fmt::format("GenerateMipsCB{} for texture {}", MipLevel, Texture->Name));
        FGenerateMipsCB* Data = GenerateMipsCB->GetData<FGenerateMipsCB>();
        Data->TexelSize = vec2(1.0f / TextureSizeX, 1.0f / TextureSizeY);

        RDGDescriptorSet* DescriptorSet = Graph.CreateDescriptorSet<FGenerateMipsCS>(0, 0);
        DescriptorSet->SetSampler("MipInSRV", Graph.CreateTextureView(RDGTextureViewDesc::CreateForMipLevel(Texture, MipLevel - 1)), Sampler);
        DescriptorSet->SetStorageImage("MipOutUAV", Graph.CreateTextureView(RDGTextureViewDesc::CreateForMipLevel(Texture, MipLevel)));
        DescriptorSet->SetUniformBuffer("GenerateMipsCB", GenerateMipsCB);

        RDGComputePassDesc PassDesc;
        PassDesc.Name = fmt::format("GenerateMips for texture {} mipmap {}", Texture->Name, MipLevel);
        PassDesc.DescriptorSets = { DescriptorSet };
        
        Graph.AddComputePass(
            PassDesc,
            [=](RHICommandList& RHICmdList)
            {
                RHICmdList.BindPipeline(PSO, EPipelineBindPoint::Compute);
                RHICmdList.BindDescriptorSets(PSO->GetPipelineLayout(), { {0, DescriptorSet->GetRHI()} }, EPipelineBindPoint::Compute);
                int32 group_count_x = Math::DivideAndRoundUp(TextureSizeX, GroupSize);
                int32 group_count_y = Math::DivideAndRoundUp(TextureSizeY, GroupSize);
                RHICmdList.DispatchCompute(group_count_x, group_count_y, 1);
            }
        );
    }
}

}