#include "GenerateMips.h"
#include "RHIStaticStates.h"
#include "Shader.h"
#include "ShaderInstance.h"
#include "UniformBuffer.h"
#include "RHICommandList.h"
#include "RenderGraphUtils.h"
#include "ComputeGenerateMips.generated.h"
#include "RasterGenerateMips.generated.h"

namespace nilou {

DECLARE_GLOBAL_SHADER(FGenerateMipsCS)
IMPLEMENT_SHADER_TYPE(FGenerateMipsCS, "/Shaders/Private/GenerateMips/ComputeGenerateMips.slang", "Main", EShaderFrequency::Compute);

DECLARE_GLOBAL_SHADER(FGenerateMipsVS)
IMPLEMENT_SHADER_TYPE(FGenerateMipsVS, "/Shaders/Private/GenerateMips/RasterGenerateMips.slang", "MainVS", EShaderFrequency::Vertex);

DECLARE_GLOBAL_SHADER(FGenerateMipsPS)
IMPLEMENT_SHADER_TYPE(FGenerateMipsPS, "/Shaders/Private/GenerateMips/RasterGenerateMips.slang", "MainPS", EShaderFrequency::Pixel);

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
    if (!Sampler)
    {
        Sampler = TStaticSamplerState<SF_Bilinear>::GetRHI();
    }

    const RDGTextureDesc& Desc = Texture->Desc;

    RHIShader *Shader = GetGlobalShader<FGenerateMipsCS>();
    RHIComputePipelineState* PSO = RHICreateComputePipelineState(static_cast<RHIComputeShader*>(Shader));

    for (int MipLevel = 1; MipLevel < Desc.NumMips; MipLevel++)
    {
        int TextureSizeX = std::max(Desc.SizeX >> MipLevel, 1u);
        int TextureSizeY = std::max(Desc.SizeY >> MipLevel, 1u);

        for (int ArrayIndex = 0; ArrayIndex < Desc.ArraySize; ArrayIndex++)
        {
            TParameterBlock<FComputeGenerateMipsInputParameters> InputParameters;
            InputParameters.MipInSRV.TextureView = Graph.CreateTextureView("MipInSRV", Texture, CreateDescForMipmap(Texture, MipLevel - 1, ArrayIndex));
            TParameterBlock<FComputeGenerateMipsOutputParameters> OutputParameters;
            OutputParameters.MipOutUAV = Graph.CreateTextureView("MipOutUAV", Texture, CreateDescForMipmap(Texture, MipLevel, ArrayIndex));
            RDGDescriptorSet* DescriptorSet = Graph.CreateDescriptorSet("GenerateMips DescriptorSet", InputParameters);
            RDGDescriptorSet* OutputDescriptorSet = Graph.CreateDescriptorSet("GenerateMips Output DescriptorSet", OutputParameters);
            RDGPassDesc PassDesc{NFormat("GenerateMips for texture \"{}\" mipmap {}", Texture->Name, MipLevel)};
            Graph.AddComputePass(
                PassDesc,
                { DescriptorSet, OutputDescriptorSet },
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
    if (!Sampler)
    {
        Sampler = TStaticSamplerState<SF_Bilinear>::GetRHI();
    }

    const RDGTextureDesc& Desc = Texture->Desc;

    for (int MipLevel = 1; MipLevel < Desc.NumMips; MipLevel++)
    {
        int TextureSizeX = std::max(Desc.SizeX >> MipLevel, 1u);
        int TextureSizeY = std::max(Desc.SizeY >> MipLevel, 1u);
        
        for (int ArrayIndex = 0; ArrayIndex < Desc.ArraySize; ArrayIndex++)
        {
            RDGRenderTargets RenderTargets;
            RenderTargets.ColorAttachments[0] = { 
                Graph.CreateTextureView("MipOutUAV", Texture, CreateDescForMipmap(Texture, MipLevel, ArrayIndex)), 
                ERenderTargetLoadAction::Clear, 
                ERenderTargetStoreAction::Store 
            };

            RHIShader *VertexShader = GetGlobalShader<FGenerateMipsVS>();
            RHIShader *PixelShader = GetGlobalShader<FGenerateMipsPS>();

            FGraphicsPipelineStateInitializer GraphicsPSOInit;
            GraphicsPSOInit.VertexShader = static_cast<RHIVertexShader*>(VertexShader);
            GraphicsPSOInit.PixelShader = static_cast<RHIPixelShader*>(PixelShader);
            GraphicsPSOInit.RTLayout = RenderTargets.GetRenderTargetLayout();
            GraphicsPSOInit.VertexDeclaration = RDGGetScreenQuadVertexDeclaration();
            RHIGraphicsPipelineState* PSO = RHICreateGraphicsPipelineState(GraphicsPSOInit);

            TParameterBlock<FRasterGenerateMipsParameters> Parameters;
            Parameters.HalfTexelSize = FVector2f(1.0f / TextureSizeX, 1.0f / TextureSizeY);
            Parameters.Level = float(MipLevel);
            Parameters.MipInSRV = { 
                .TextureView = Graph.CreateTextureView("MipInSRV", Texture, CreateDescForMipmap(Texture, MipLevel - 1, ArrayIndex)), 
                .SamplerState = Sampler 
            };
            RDGDescriptorSet* DescriptorSet = Graph.CreateDescriptorSet("GenerateMips DescriptorSet", Parameters);

            RDGBuffer* ScreenQuadVertexBuffer = RDGGetScreenQuadVertexBuffer(Graph);
            RDGBuffer* ScreenQuadIndexBuffer = RDGGetScreenQuadIndexBuffer(Graph);

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

                    RHICmdList.BindGraphicsPipelineState(PSO);
                    RHICmdList.BindDescriptorSets(PSO->GetPipelineLayout(), { {0, DescriptorSet->GetRHI()} }, EPipelineBindPoint::Graphics);
                    RHICmdList.BindVertexBuffer(0, ScreenQuadVertexBuffer->GetRHI(), 0);
                    RHICmdList.BindIndexBuffer(ScreenQuadIndexBuffer->GetRHI(), 0);
                    RHICmdList.DrawIndexed(6, 1, 0, 0, 0);
                }
            );
        }
    }
}

}