#pragma once
#include <vector>

#include "ShaderType.h"
#include "RenderGraphResources.h"
#include "RHIResources.h"
#include "Templates/TypeTraits.h"

namespace nilou {

using PassHandle = int32;
class FDynamicRHI;

class FRDGPassNode;
class FRDGResourceNode
{
public:
    FRDGPassNode* InPassNode;
};

class FRDGPassNode
{
public:
    std::vector<FRDGResourceNode*> InResourceNodes;
    std::vector<FRDGResourceNode*> OutResourceNodes;
    std::vector<FRDGResourceNode*> InOutResourceNodes;

    bool bCulled = true;
    bool bForceExecute = false;

};

template <typename TDesc, typename T>
class TRDGAllocator
{

public:

    T* Alloc(const TDesc& Desc)
    {
        auto begin = Pool.equal_range(Desc).first;
        if (begin != Pool.end())
        {
            T* obj = begin->second;
            Pool.erase(begin);
            return obj;
        }
        return new T(Desc);
    }

    void Release(const TDesc& Desc, T* Obj)
    {
        Pool.insert({Desc, Obj});
    }

    static TRDGAllocator& Get()
    {
        static TRDGAllocator Allocator;
        return Allocator;
    }

private:

	std::unordered_multimap<TDesc, T*> Pool;

};

using FRDGTextureAllocator = TRDGAllocator<FRDGTextureDesc, FRDGTexture>;
using FRDGBufferAllocator = TRDGAllocator<FRDGBufferDesc, FRDGBuffer>;
using FRDGUniformBufferAllocator = TRDGAllocator<FRDGUniformBufferDesc, FRDGUniformBuffer>;

template <typename TDesc, typename T>
class TRDGResourceRegistry
{
public:

    TRDGResourceRegistry() = default;
    ~TRDGResourceRegistry()
    {
        auto& Allocator = TRDGAllocator<TDesc, T>::Get();
        for (auto& [Desc, Obj] : TrackedResources)
        {
            Allocator.Release(Desc, Obj);
        }
    }

    void Add(const TDesc& Desc, T* Obj)
    {
        TrackedResources.push_back({Desc, Obj});
    }

private:

    std::vector<std::pair<TDesc, T*>> TrackedResources;
};

using FRDGTextureRegistry = TRDGResourceRegistry<FRDGTextureDesc, FRDGTexture>;
using FRDGBufferRegistry = TRDGResourceRegistry<FRDGBufferDesc, FRDGBuffer>;
using FRDGUniformBufferRegistry = TRDGResourceRegistry<FRDGUniformBufferDesc, FRDGUniformBuffer>;class FRDGPassBuilder
{
public:

    FRDGPassBuilder();

    FRDGPassBuilder& Read(const std::string& ParamName, FRDGUniformBuffer* UniformBuffer) { return *this; }

    FRDGPassBuilder& Read(const std::string& ParamName, FRDGBuffer* Buffer) { return *this; }

    FRDGPassBuilder& Read(const std::string& ParamName, FRDGTexture* Texture, const RHITextureParams& Sampler=RHITextureParams::DefaultParams) { return *this; }

    FRDGPassBuilder& Write(const std::string& ParamName, FRDGBuffer* Buffer) { return *this; }

    FRDGPassBuilder& Write(const std::string& ParamName, FRDGTexture* Texture) { return *this; }

    FRDGPassBuilder& Write(int32 Location, FRDGTexture* Texture) { return *this; }

protected:

    FRDGPassNode* Node;
};

class FRDGComputePassBuilder : public FRDGPassBuilder
{
public:

    FRDGComputePassBuilder();

    FRDGComputePassBuilder& Read(const std::string& ParamName, FRDGUniformBuffer* UniformBuffer) { return *this; }

    FRDGComputePassBuilder& Read(const std::string& ParamName, FRDGBuffer* Buffer) { return *this; }

    FRDGComputePassBuilder& Read(const std::string& ParamName, FRDGTexture* Texture, const RHITextureParams& Sampler=RHITextureParams::DefaultParams) { return *this; }

    FRDGComputePassBuilder& Write(const std::string& ParamName, FRDGBuffer* Buffer) { return *this; }

    FRDGComputePassBuilder& Write(const std::string& ParamName, FRDGTexture* Texture) { return *this; }

};

class FRDGGraphicsPassBuilder : public FRDGPassBuilder
{
public:

    FRDGGraphicsPassBuilder();

    FRDGGraphicsPassBuilder& Read(const std::string& ParamName, FRDGUniformBuffer* UniformBuffer) { return *this; }

    FRDGGraphicsPassBuilder& Read(const std::string& ParamName, FRDGBuffer* Buffer) { return *this; }

    FRDGGraphicsPassBuilder& Read(const std::string& ParamName, FRDGTexture* Texture, const RHITextureParams& Sampler=RHITextureParams::DefaultParams) { return *this; }

    FRDGGraphicsPassBuilder& Write(const std::string& ParamName, FRDGBuffer* Buffer) { return *this; }

    FRDGGraphicsPassBuilder& Write(const std::string& ParamName, FRDGTexture* Texture) { return *this; }

};

class FRDGPresentPassBuilder : public FRDGPassBuilder
{
public:

    FRDGPresentPassBuilder();

    FRDGPresentPassBuilder& Write(FRDGTexture* Texture) { return *this; }

};

class FRenderGraph
{
public:

    friend class FRDGBuilder;

    static FRDGTextureRef CreatePersistentTexture(const FRDGTextureDesc& TextureDesc);

    static FRDGBufferRef CreatePersistentBuffer(const FRDGBufferDesc& Desc);

    static FRDGUniformBufferRef CreatePersistentUniformBuffer(const FRDGUniformBufferDesc& Desc);

    static FRenderGraph* RDG;

    FRDGTexture* CreateTexture(const std::string& Name, const FRDGTextureDesc& TextureDesc);

    FRDGBuffer* CreateBuffer(const FRDGBufferDesc& Desc);

    FRDGUniformBuffer* CreateUniformBuffer(const FRDGUniformBufferDesc& Desc);

    PassHandle AddComputePass(const std::function<void(FRDGComputePassBuilder&)>& Setup, const std::function<void(FDynamicRHI*)>& Executor);

    PassHandle AddGraphicsPass(const std::function<void(FRDGGraphicsPassBuilder&)>& Setup, const std::function<void(FDynamicRHI*)>& Executor);

    PassHandle AddPresentPass(const std::function<void(FRDGPresentPassBuilder&)>& Setup, const std::function<void(FDynamicRHI*)>& Executor);

    void Start();

    void CleanUp();

    void Cull();

    void Compile();

    void Execute();

protected:

private:

    FRDGPassNode* PresentPass;

    std::vector<FRDGPassNode*> Passes;

    std::vector<FRDGResourceNode*> Resources;

    FRDGTextureAllocator& TextureAllocator;

    FRDGTextureRegistry Textures{};

    FRDGBufferAllocator& BufferAllocator;

    FRDGBufferRegistry Buffers{};

    FRDGUniformBufferAllocator& UniformBufferAllocator;

    FRDGUniformBufferRegistry UniformBuffers{};

};

}