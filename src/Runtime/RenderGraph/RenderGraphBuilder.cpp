#include "RenderGraphBuilder.h"

namespace nilou {

FRDGPassBuilder::FRDGPassBuilder()
{
    Node = new FRDGPassNode;
}

void FRDGComputePassBuilder::SetComputeShader(const FShaderPermutationParameters& Params)
{
    
}

FRDGTexture* FRDGBuilder::CreateTexture(const FRDGTextureDesc& TextureDesc)
{
    FRDGTexture* Texture = TextureAllocator.Alloc(TextureDesc);
    Textures.Add(TextureDesc, Texture);
    Texture->Node = Graph.Resources.emplace_back();
    return Texture;
}

FRDGBuffer* FRDGBuilder::CreateBuffer(const FRDGBufferDesc& Desc)
{
    FRDGBuffer* Buffer = BufferAllocator.Alloc(Desc);
    Buffers.Add(Desc, Buffer);
    Buffer->Node = Graph.Resources.emplace_back();
    return Buffer;
}

FRDGUniformBuffer* FRDGBuilder::CreateUniformBuffer(const FRDGUniformBufferDesc& Desc)
{
    FRDGUniformBuffer* Buffer = UniformBufferAllocator.Alloc(Desc);
    UniformBuffers.Add(Desc, Buffer);
    Buffer->Node = Graph.Resources.emplace_back();
    return Buffer;
}

FRDGTexture* FRDGBuilder::RegisterExternalTexture(RHITexture* TextureRHI)
{
    auto Found = ExternalTextures.find(TextureRHI);
    if (Found != ExternalTextures.end())
        return Found->second;
    
    FRDGTextureDesc Desc;
    Desc.TextureType = TextureRHI->GetTextureType();
    Desc.ArraySize = TextureRHI->GetNumLayers();
    Desc.SizeX = TextureRHI->GetSizeX();
    Desc.SizeY = TextureRHI->GetSizeY();
    Desc.SizeZ = TextureRHI->GetSizeZ();
    Desc.Format = TextureRHI->GetFormat();
    Desc.NumMips = TextureRHI->GetNumMips();
    FRDGTexture* Texture = CreateTexture(Desc);
    Texture->Name = TextureRHI->GetName();
    Texture->ResourceRHI = TextureRHI;
    ExternalTextures[TextureRHI] = Texture;
    return Texture;
    
}

PassHandle FRDGBuilder::AddComputePass(const std::function<void(FRDGComputePassBuilder&)>& Setup)
{
    return 0;
}

}