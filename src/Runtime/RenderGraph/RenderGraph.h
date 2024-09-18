#pragma once
#include <vector>

#include "ShaderType.h"
#include "RenderGraphResources.h"
#include "RHIResources.h"
#include "Templates/TypeTraits.h"
#include "RenderGraphDescriptorSet.h"

namespace nilou {

using PassHandle = int32;
class FDynamicRHI;
class RHICommandList;

class RDGPassNode;
class RDGResourceNode
{
public:
    RDGPassNode* InPassNode;
};

class RDGPassNode
{
public:
    std::vector<RDGResourceNode*> InResourceNodes;
    std::vector<RDGResourceNode*> OutResourceNodes;

    bool bCulled = true;
    bool bForceExecute = false;

};

enum class ERDGTextureUsage
{
    None,
    RenderTarget,
    ShaderResource,
};

class RDGPassBuilder
{
public:

    RDGPassBuilder();

    RDGPassBuilder& Read(RDGBuffer* Buffer) { return *this; }

    RDGPassBuilder& Read(RDGTexture* Texture, ERDGTextureUsage TextureUsage) { return *this; }

    RDGPassBuilder& Write(RDGBuffer* Buffer) { return *this; }

    RDGPassBuilder& Write(RDGTexture* Texture, ERDGTextureUsage TextureUsage) { return *this; }

protected:

    RDGPassNode* Node;

};

struct RDGPassDesc
{
    std::string Name;

    // If this pass should never be culled.
    bool bNeverCull = false;
};

struct RDGGraphicsPassDesc : public RDGPassDesc
{
    // The descriptor sets used by this graphics pass. Note: The place of each set in the vector doesn't matter.
    std::vector<RDGDescriptorSet*> DescriptorSets;

    // The render targets used by this graphics pass.
    RDGFramebuffer RenderTargets;
};

struct RDGComputePassDesc : public RDGPassDesc
{
    // The descriptor sets used by this compute pass. Note: The place of each set in the vector doesn't matter.
    std::vector<RDGDescriptorSet*> DescriptorSets;
};

struct RDGCopyPassDesc : public RDGPassDesc
{
    RDGBuffer* Source;
    RDGTexture* Destination;
};

class RenderGraph
{
public:

    friend class RDGBuilder;

    static RDGTextureRef CreatePersistentTexture(const std::string& Name, const RDGTextureDesc& TextureDesc);

    static RDGTextureViewRef CreatePersistentTextureView(const RDGTextureViewDesc& TextureViewDesc);

    static RDGBufferRef CreatePersistentBuffer(const std::string& Name, const RDGBufferDesc& Desc);

    static RDGDescriptorSetRef CreatePersistentDescriptorSet(RHIDescriptorSetLayout* Layout);

    RDGTexture* CreateTexture(const std::string& Name, const RDGTextureDesc& TextureDesc);

    RDGTextureView* CreateTextureView(const RDGTextureViewDesc& TextureViewDesc);

    RDGBuffer* CreateBuffer(const std::string& Name, const RDGBufferDesc& Desc);
    
    template<class TUniformBufferType>
    RDGBuffer* CreateUniformBuffer(const std::string& Name)
    {
        RDGBufferDesc Desc;
        Desc.Stride = 0;
        Desc.Size = sizeof(TUniformBufferType);
        return CreateBuffer(Name, Desc);
    }

    RDGDescriptorSet* CreateDescriptorSet(RHIDescriptorSetLayout* Layout);

    template<class TShaderType>
    RDGDescriptorSet* CreateDescriptorSet(int32 PermutationId, uint32 SetIndex)
    {
        return CreateDescriptorSet(TShaderType::GetDescriptorSetLayout(PermutationId, SetIndex));
    }



    // Add a graphics pass to the render graph
    PassHandle AddGraphicsPass(
        const RDGGraphicsPassDesc& PassDesc,
        const std::function<void(RHICommandList&)>& Executor);

    // Add a compute pass to the render graph
    PassHandle AddComputePass(
        const RDGComputePassDesc& PassDesc,
        const std::function<void(RHICommandList&)>& Executor);

    PassHandle AddCopyPass(
        const RDGCopyPassDesc& PassDesc,
        const std::function<void(RHICommandList&)>& Executor);

    void Start();

    void CleanUp();

    void Cull();

    void Compile();

    void Execute();

protected:

    static RDGTextureView* CreatePersistentTextureViewInternal(const RDGTextureViewDesc& TextureViewDesc);

private:

    RDGPassNode* PresentPass;

    std::vector<RDGPassNode*> Passes;

    std::vector<RDGResourceNode*> Resources;
    
    std::vector<RDGFramebuffer> Framebuffers;

    // Resources whose lifetime is managed by the render graph
    std::vector<RDGTextureRef> Textures;
    std::vector<RDGBufferRef> Buffers;
    std::vector<RDGDescriptorSetRef> DescriptorSets;

    static std::map<RHIDescriptorSetLayout*, RDGDescriptorSetPool> DescriptorSetPools;
};

}