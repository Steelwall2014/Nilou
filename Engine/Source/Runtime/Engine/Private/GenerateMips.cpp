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
            auto InputParams = Graph.CreateParameterBlock<FComputeGenerateMipsInputParameters>("GenerateMips Input ParamBlock");
            InputParams->MipInSRV = {
                .TextureView = Graph.CreateTextureView("MipInSRV", Texture, CreateDescForMipmap(Texture, MipLevel - 1, ArrayIndex)),
                .SamplerState = Sampler
            };
            Graph.UpdateParameterBlock(InputParams);
            auto OutputParams = Graph.CreateParameterBlock<FComputeGenerateMipsOutputParameters>("GenerateMips Output ParamBlock");
            OutputParams->MipOutUAV = Graph.CreateTextureView("MipOutUAV", Texture, CreateDescForMipmap(Texture, MipLevel, ArrayIndex));
            Graph.UpdateParameterBlock(OutputParams);
            RDGPassDesc PassDesc{NFormat("GenerateMips for texture \"{}\" mipmap {}", Texture->Name, MipLevel)};
            Graph.AddComputePass(
                PassDesc,
                [=](FRDGPass* Pass)
                {
                    Pass->AddParameterBlock(InputParams);
                    Pass->AddParameterBlock(OutputParams);
                },
                [=](RHICommandList& RHICmdList)
                {
                    RHICmdList.BindComputePipelineState(PSO);
                    auto PipelineLayout = PSO->GetPipelineLayout();
                    int32 InputParamsSetIndex = PipelineLayout->GetSetIndex("InputParams");
                    int32 OutputParamsSetIndex = PipelineLayout->GetSetIndex("OutputParams");
                    RHICmdList.BindDescriptorSets(
                        PipelineLayout,
                        {{InputParamsSetIndex, InputParams->DescriptorSet->GetRHI()}, {OutputParamsSetIndex, OutputParams->DescriptorSet->GetRHI()}},
                        EPipelineBindPoint::Compute);
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

            auto Params = Graph.CreateParameterBlock<FRasterGenerateMipsParameters>("GenerateMips ParamBlock");
            Params->GetNonOpaqueFields().HalfTexelSize = FVector2f(1.0f / TextureSizeX, 1.0f / TextureSizeY);
            Params->GetNonOpaqueFields().Level = float(MipLevel);
            Params->MipInSRV = { 
                .TextureView = Graph.CreateTextureView("MipInSRV", Texture, CreateDescForMipmap(Texture, MipLevel - 1, ArrayIndex)), 
                .SamplerState = Sampler 
            };
            Graph.UpdateParameterBlock(Params);

            RDGBuffer* ScreenQuadVertexBuffer = RDGGetScreenQuadVertexBuffer(Graph);
            RDGBuffer* ScreenQuadIndexBuffer = RDGGetScreenQuadIndexBuffer(Graph);

            RDGPassDesc PassDesc{NFormat("GenerateMips for texture \"{}\" mipmap {}", Texture->Name, MipLevel)};
            PassDesc.bNeverCull = true;
            Graph.AddGraphicsPass(
                PassDesc,
                RenderTargets,
                { ScreenQuadIndexBuffer },
                { ScreenQuadVertexBuffer },
                [=](FRDGPass* Pass)
                {
                    Pass->AddParameterBlock(Params);
                },
                [=](RHICommandList& RHICmdList)
                {
                    RHICmdList.BindGraphicsPipelineState(PSO);
                    auto PipelineLayout = PSO->GetPipelineLayout();
                    int32 ParamsSetIndex = PipelineLayout->GetSetIndex("Params");
                    RHICmdList.BindDescriptorSets(
                        PipelineLayout,
                        {{ParamsSetIndex, Params->DescriptorSet->GetRHI()}},
                        EPipelineBindPoint::Graphics);
                    RHICmdList.BindVertexBuffer(0, ScreenQuadVertexBuffer->GetRHI(), 0);
                    RHICmdList.BindIndexBuffer(ScreenQuadIndexBuffer->GetRHI(), 0);
                    RHICmdList.DrawIndexed(6, 1, 0, 0, 0);
                }
            );
        }
    }
}

}