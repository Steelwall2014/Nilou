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

RDGTextureViewDesc CreateDescForMipmap(RDGTexture* Texture, int MipmapIndex)
{
    RDGTextureViewDesc Desc;
    Desc.ViewType = Texture->Desc.TextureType;
    Desc.Format = Texture->Desc.Format;
    Desc.BaseArrayLayer = 0;
    Desc.LayerCount = Texture->Desc.ArraySize;
    Desc.BaseMipLevel = MipmapIndex;
    Desc.LevelCount = 1;
    return Desc;
}

void FGenerateMips::Execute(RenderGraph& Graph, RDGTexture* Texture, RHISamplerState* Sampler)
{
    Ncheck(Texture);
    Ncheck(Sampler);

    const RDGTextureDesc& Desc = Texture->Desc;

    FShaderInstance* Shader = GetGlobalShader<FGenerateMipsCS>();
    RHIComputePipelineState* PSO = RHICreateComputePipelineState(Shader->GetComputeShaderRHI());

    for (int MipLevel = 1; MipLevel < Desc.NumMips; MipLevel++)
    {
        int TextureSizeX = std::max(Desc.SizeX >> MipLevel, 1u);
        int TextureSizeY = std::max(Desc.SizeY >> MipLevel, 1u);

        TRDGUniformBuffer<FGenerateMipsCB>* GenerateMipsCB = Graph.CreateUniformBuffer<FGenerateMipsCB>(NFormat("GenerateMipsCB{} for texture {}", MipLevel, Texture->Name));
        FGenerateMipsCB& Data = GenerateMipsCB->GetData();
        Data.TexelSize = vec2(1.0f / TextureSizeX, 1.0f / TextureSizeY);

        RDGDescriptorSet* DescriptorSet = Graph.CreateDescriptorSet<FGenerateMipsCS>(0, 0);
        DescriptorSet->SetSampler("MipInSRV", Graph.CreateTextureView("MipInSRV", Texture, CreateDescForMipmap(Texture, MipLevel - 1)), Sampler);
        DescriptorSet->SetStorageImage("MipOutUAV", Graph.CreateTextureView("MipOutUAV", Texture, CreateDescForMipmap(Texture, MipLevel)), ERHIAccess::ShaderResourceWrite);
        DescriptorSet->SetUniformBuffer("GenerateMipsCB", GenerateMipsCB);

        RDGPassDesc PassDesc{NFormat("GenerateMips for texture {} mipmap {}", Texture->Name, MipLevel)};
        Graph.AddComputePass(
            PassDesc,
            { DescriptorSet },
            [=](RHICommandList& RHICmdList)
            {
                RHICmdList.BindComputePipelineState(PSO);
                RHICmdList.BindDescriptorSets(PSO->GetPipelineLayout(), { {0, DescriptorSet->GetRHI()} }, EPipelineBindPoint::Compute);
                int32 group_count_x = FMath::DivideAndRoundUp(TextureSizeX, GroupSize);
                int32 group_count_y = FMath::DivideAndRoundUp(TextureSizeY, GroupSize);
                RHICmdList.DispatchCompute(group_count_x, group_count_y, 1);
            }
        );
    }
}

}