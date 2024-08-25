#include "RenderGraph.h"
#include "DynamicRHI.h"

namespace nilou {

RDGTextureRef RenderGraph::CreatePersistentTexture(const std::string& Name, const RDGTextureDesc& Desc)
{
    std::shared_ptr<RDGTexture> Texture = std::make_shared<RDGTexture>(Desc);
    Texture->Name = Name;
    Texture->bIsPersistent = true;
    switch (Desc.TextureType)
    {
    case ETextureDimension::Texture2D:
        Texture->ResourceRHI = RHICreateTexture2D(Name, Desc.Format, Desc.NumMips, Desc.SizeX, Desc.SizeY, Desc.TexCreateFlags);
        break;
    case ETextureDimension::Texture2DArray:
        Texture->ResourceRHI = RHICreateTexture2DArray(Name, Desc.Format, Desc.NumMips, Desc.SizeX, Desc.SizeY, Desc.ArraySize, Desc.TexCreateFlags);
        break;
    case ETextureDimension::Texture3D:
        Texture->ResourceRHI = RHICreateTexture3D(Name, Desc.Format, Desc.NumMips, Desc.SizeX, Desc.SizeY, Desc.SizeZ, Desc.TexCreateFlags);
        break;
    case ETextureDimension::TextureCube:
        Texture->ResourceRHI = RHICreateTextureCube(Name, Desc.Format, Desc.NumMips, Desc.SizeX, Desc.SizeY, Desc.TexCreateFlags);
        break;
    default:
        Ncheckf(false, "Invalid texture type");
    };
    return Texture;
}

RDGBufferRef RenderGraph::CreatePersistentBuffer(const std::string& Name, const RDGBufferDesc& BufferDesc)
{
    std::shared_ptr<RDGBuffer> Buffer = std::make_shared<RDGBuffer>(BufferDesc);
    Buffer->Name = Name;
    Buffer->bIsPersistent = true;
    Buffer->ResourceRHI = RHICreateBuffer(BufferDesc.Stride, BufferDesc.Size, EBufferUsageFlags::None, nullptr);
    return Buffer;
}

RDGDescriptorSetRef RenderGraph::CreatePersistentDescriptorSet(RHIDescriptorSetLayout* Layout)
{
    if (DescriptorSetPools.find(Layout) == DescriptorSetPools.end())
    {
        DescriptorSetPools[Layout] = RDGDescriptorSetPool(Layout);
    }

    RDGDescriptorSetPool& Pool = DescriptorSetPools[Layout];
    RDGDescriptorSetRef DescriptorSet = Pool.Allocate();
    return DescriptorSet;
}

RDGTexture* RenderGraph::CreateTexture(const std::string& Name, const RDGTextureDesc& Desc)
{
    std::shared_ptr<RDGTexture> Texture = std::make_shared<RDGTexture>(Desc);
    Texture->Name = Name;
    Textures.push_back(Texture);
    return Texture.get();
}

// RDGTextureView* RenderGraph::CreateTextureView(const std::string& Name, RDGTexture* Texture, const RDGTextureViewDesc& ViewDesc)
// {
//     RDGTextureDesc Desc;
//     Desc.SizeX = Texture->Desc.SizeX >> ViewDesc.LevelCount;
//     Desc.SizeY = Texture->Desc.SizeY >> ViewDesc.LevelCount;
//     Desc.SizeZ = Texture->Desc.SizeZ >> ViewDesc.LevelCount;
//     Desc.ArraySize = ViewDesc.LayerCount;
//     Desc.NumMips = ViewDesc.LevelCount;
//     Desc.Format = ViewDesc.Format;
//     Desc.TexCreateFlags = Texture->Desc.TexCreateFlags;
//     Desc.TextureType = ViewDesc.ViewType;

//     std::shared_ptr<RDGTextureView> TextureView = std::make_shared<RDGTextureView>(Desc, Texture, ViewDesc);
//     RDGTextureView* TextureViewPtr = TextureView.get();
//     Textures.push_back(TextureView);

//     return TextureViewPtr;
// }

RDGBuffer* RenderGraph::CreateBuffer(const std::string& Name, const RDGBufferDesc& Desc)
{
    std::shared_ptr<RDGBuffer> Buffer = std::make_shared<RDGBuffer>(Desc);
    Buffer->Name = Name;
    Buffers.push_back(Buffer);
    return Buffer.get();
}

RDGTextureSRV* RenderGraph::CreateSRV(const RDGTextureSRVDesc& Desc)
{
    
}
RDGBufferSRV* RenderGraph::CreateSRV(const RDGBufferSRVDesc& Desc)
{
    
}

RDGTextureUAV* RenderGraph::CreateUAV(const RDGTextureUAVDesc& Desc)
{
    
}
RDGBufferUAV* RenderGraph::CreateUAV(const RDGBufferUAVDesc& Desc)
{
    
}

RDGDescriptorSet* RenderGraph::CreateDescriptorSet(RHIDescriptorSetLayout* Layout)
{
    if (DescriptorSetPools.find(Layout) == DescriptorSetPools.end())
    {
        DescriptorSetPools[Layout] = RDGDescriptorSetPool(Layout);
    }

    RDGDescriptorSetPool& Pool = DescriptorSetPools[Layout];
    RDGDescriptorSetRef DescriptorSet = Pool.Allocate();
    AllocatedDescriptorSets.push_back(DescriptorSet);
    return DescriptorSet.get();
}

void RenderGraph::Cull()
{
    for (RDGPassNode* Pass : Passes)
        Pass->bCulled = true;
    std::map<RDGPassNode*, int> indegrees;
    std::queue<RDGPassNode*> q;
    std::vector<RDGPassNode*> Passes_0indegree;
    q.push(PresentPass);
    while (!q.empty())
    {
        RDGPassNode* node = q.front(); q.pop();
        node->bCulled = false;
        indegrees[node] = 0;
        for (RDGResourceNode* InResourceNode : node->InResourceNodes)
        {
            if (InResourceNode->InPassNode)
            {
                indegrees[node]++;
                q.push(InResourceNode->InPassNode);
            }
        }
        if (indegrees[node] == 0)
        {
            Passes_0indegree.push_back(node);
        }
    }

}

}