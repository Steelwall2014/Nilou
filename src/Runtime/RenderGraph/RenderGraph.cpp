#include "RenderGraph.h"

namespace nilou {

std::vector<FRDGTexture*> PendingFreeTextures;
std::vector<FRDGBuffer*> PendingFreeBuffers;
std::vector<FRDGUniformBuffer*> PendingFreeUniformBuffers;

FRDGTextureRef FRenderGraph::CreatePersistentTexture(const FRDGTextureDesc& TextureDesc)
{
    FRDGTextureRef Texture(new FRDGTexture(TextureDesc), 
        [](FRDGTexture* Obj) {
            PendingFreeTextures.push_back(Obj);
        });
    Texture->bIsPersistent = true;
    return Texture;
}

FRDGBufferRef FRenderGraph::CreatePersistentBuffer(const FRDGBufferDesc& BufferDesc)
{
    FRDGBufferRef Buffer(new FRDGBuffer(BufferDesc), 
        [](FRDGBuffer* Obj) {
            PendingFreeBuffers.push_back(Obj);
        });
    Buffer->bIsPersistent = true;
    return Buffer;
}

FRDGUniformBufferRef FRenderGraph::CreatePersistentUniformBuffer(const FRDGUniformBufferDesc& BufferDesc)
{
    FRDGUniformBufferRef Buffer(new FRDGUniformBuffer(BufferDesc), 
        [](FRDGUniformBuffer* Obj) {
            PendingFreeUniformBuffers.push_back(Obj);
        });
    Buffer->bIsPersistent = true;
    return Buffer;
}

FRDGTexture* FRenderGraph::CreateTexture(const std::string& Name, const FRDGTextureDesc& TextureDesc)
{
    FRDGTexture* Texture = TextureAllocator.Alloc(TextureDesc);
    Textures.Add(TextureDesc, Texture);
    Texture->Node = Resources.emplace_back();
    Texture->Name = Name;
    return Texture;
}

FRDGBuffer* FRenderGraph::CreateBuffer(const FRDGBufferDesc& Desc)
{
    FRDGBuffer* Buffer = BufferAllocator.Alloc(Desc);
    Buffers.Add(Desc, Buffer);
    Buffer->Node = Resources.emplace_back();
    return Buffer;
}

FRDGUniformBuffer* FRenderGraph::CreateUniformBuffer(const FRDGUniformBufferDesc& Desc)
{
    FRDGUniformBuffer* Buffer = UniformBufferAllocator.Alloc(Desc);
    UniformBuffers.Add(Desc, Buffer);
    Buffer->Node = Resources.emplace_back();
    return Buffer;
}

void FRenderGraph::Cull()
{
    for (FRDGPassNode* Pass : Passes)
        Pass->bCulled = true;
    std::map<FRDGPassNode*, int> indegrees;
    std::queue<FRDGPassNode*> q;
    std::vector<FRDGPassNode*> Passes_0indegree;
    q.push(PresentPass);
    while (!q.empty())
    {
        FRDGPassNode* node = q.front(); q.pop();
        node->bCulled = false;
        indegrees[node] = 0;
        for (FRDGResourceNode* InResourceNode : node->InResourceNodes)
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