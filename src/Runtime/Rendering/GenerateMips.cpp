#include "GenerateMips.h"
#include "Shader.h"
#include "ShaderInstance.h"
#include "UniformBuffer.h"
#include "RHICommandList.h"
#include "RenderGraphUtils.h"

namespace nilou {

DECLARE_GLOBAL_SHADER(FGenerateMipsCS)
IMPLEMENT_SHADER_TYPE(FGenerateMipsCS, "/Shaders/GlobalShaders/ComputeGenerateMips.glsl", EShaderFrequency::SF_Compute, Global);

DECLARE_GLOBAL_SHADER(FGenerateMipsVS)
IMPLEMENT_SHADER_TYPE(FGenerateMipsVS, "/Shaders/GlobalShaders/RasterGenerateMipsVertexShader.glsl", EShaderFrequency::SF_Vertex, Global);

class FGenerateMipsPS : public FGlobalShader
{
public:
    DECLARE_SHADER_TYPE()

    BEGIN_UNIFORM_BUFFER_STRUCT(FParameters)
        SHADER_PARAMETER(vec2, HalfTexelSize)
        SHADER_PARAMETER(float, Level)
    END_UNIFORM_BUFFER_STRUCT()
    
};
IMPLEMENT_SHADER_TYPE(FGenerateMipsPS, "/Shaders/GlobalShaders/RasterGenerateMipsPixelShader.glsl", EShaderFrequency::SF_Pixel, Global);

constexpr int GroupSize = 8;

RDGTextureViewDesc CreateDescForMipmap(RDGTexture* Texture, int MipmapIndex, int ArrayIndex)
{
    RDGTextureViewDesc Desc;
    Desc.ViewType = ETextureDimension::Texture2D;
    Desc.Format = Texture->Desc.Format;
    Desc.BaseArrayLayer = ArrayIndex;
    Desc.LayerCount = 1;
    Desc.BaseMipLevel = MipmapIndex;
    Desc.LevelCount = 1;
    return Desc;
}

void FGenerateMips::Execute(RenderGraph& Graph, RDGTexture* Texture, RHISamplerState* Sampler)
{
    ExecuteRaster(Graph, Texture, Sampler);
}

void FGenerateMips::ExecuteCompute(RenderGraph& Graph, RDGTexture* Texture, RHISamplerState* Sampler)
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

        for (int ArrayIndex = 0; ArrayIndex < Desc.ArraySize; ArrayIndex++)
        {
            RDGDescriptorSet* DescriptorSet = Graph.CreateDescriptorSet<FGenerateMipsCS>(0, 0);
            DescriptorSet->SetSampler("MipInSRV", Graph.CreateTextureView("MipInSRV", Texture, CreateDescForMipmap(Texture, MipLevel - 1, ArrayIndex)), Sampler);
            DescriptorSet->SetStorageImage("MipOutUAV", Graph.CreateTextureView("MipOutUAV", Texture, CreateDescForMipmap(Texture, MipLevel, ArrayIndex)), ERHIAccess::ShaderResourceWrite);

            RDGPassDesc PassDesc{NFormat("GenerateMips for texture \"{}\" mipmap {}", Texture->Name, MipLevel)};
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

void FGenerateMips::ExecuteRaster(RenderGraph& Graph, RDGTexture* Texture, RHISamplerState* Sampler)
{
    Ncheck(Texture);
    Ncheck(Sampler);

    const RDGTextureDesc& Desc = Texture->Desc;

    for (int MipLevel = 1; MipLevel < Desc.NumMips; MipLevel++)
    {
        int TextureSizeX = std::max(Desc.SizeX >> MipLevel, 1u);
        int TextureSizeY = std::max(Desc.SizeY >> MipLevel, 1u);
        
        for (int ArrayIndex = 0; ArrayIndex < Desc.ArraySize; ArrayIndex++)
        {
            RDGDescriptorSet* DescriptorSet = Graph.CreateDescriptorSet<FGenerateMipsPS>(0, 0);
            DescriptorSet->SetSampler("MipInSRV", Graph.CreateTextureView("MipInSRV", Texture, CreateDescForMipmap(Texture, MipLevel - 1, ArrayIndex)), Sampler);
            FGenerateMipsPS::FParameters Parameters;
            Parameters.HalfTexelSize = vec2(1.0f / TextureSizeX, 1.0f / TextureSizeY);
            Parameters.Level = float(MipLevel);
            RDGBuffer* ParametersBuffer = CreateUniformBuffer(Graph, Parameters);
            DescriptorSet->SetUniformBuffer("FParameters", ParametersBuffer);

            RDGRenderTargets RenderTargets;
            RenderTargets.ColorAttachments[0] = Graph.CreateTextureView("MipOutUAV", Texture, CreateDescForMipmap(Texture, MipLevel, ArrayIndex));

            RDGBuffer* ScreenQuadVertexBuffer = GetScreenQuadVertexBuffer(Graph);
            RDGBuffer* ScreenQuadIndexBuffer = GetScreenQuadIndexBuffer(Graph);

            RDGPassDesc PassDesc{NFormat("GenerateMips for texture \"{}\" mipmap {}", Texture->Name, MipLevel)};
            PassDesc.bNeverCull = true;
            Graph.AddGraphicsPass(
                PassDesc,
                RenderTargets,
                { ScreenQuadIndexBuffer },
                { ScreenQuadVertexBuffer },
                { DescriptorSet },
                [=](RHICommandList& RHICmdList)
                {
                    FShaderInstance* VertexShader = GetGlobalShader<FGenerateMipsVS>();
                    FShaderInstance* PixelShader = GetGlobalShader<FGenerateMipsPS>();

                    FGraphicsPipelineStateInitializer GraphicsPSOInit;
                    GraphicsPSOInit.VertexShader = VertexShader->GetVertexShaderRHI();
                    GraphicsPSOInit.PixelShader = PixelShader->GetPixelShaderRHI();
                    GraphicsPSOInit.RTLayout = RenderTargets.GetRenderTargetLayout();
                    GraphicsPSOInit.VertexDeclaration = GetScreenQuadVertexDeclaration();
                    RHIGraphicsPipelineState* PSO = RHICreateGraphicsPipelineState(GraphicsPSOInit);

                    RHICmdList.BindGraphicsPipelineState(PSO);
                    RHICmdList.BindDescriptorSets(PSO->GetPipelineLayout(), { {0, DescriptorSet->GetRHI()} }, EPipelineBindPoint::Graphics);
                    RHICmdList.BindVertexBuffer(0, ScreenQuadVertexBuffer->GetRHI(), 0);
                    RHICmdList.BindIndexBuffer(ScreenQuadIndexBuffer->GetRHI(), 0);
                    RHICmdList.DrawIndexed(3, 1, 0, 0, 0);
                }
            );
        }
    }
}

}