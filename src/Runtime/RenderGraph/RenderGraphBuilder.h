#pragma once
#include <unordered_map>
#include <functional>

#include "RenderGraph.h"
#include "RenderGraphResources.h"
#include "RHIResources.h"
#include "Templates/TypeTraits.h"

namespace nilou {

class FDynamicRHI;

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
using FRDGUniformBufferRegistry = TRDGResourceRegistry<FRDGUniformBufferDesc, FRDGUniformBuffer>;

class FRDGPassBuilder
{
public:

    FRDGPassBuilder();

    void SetShaderUniformBuffer(const std::string& ParamName, FRDGUniformBuffer* UniformBuffer);

    void SetShaderBuffer(const std::string& ParamName, FRDGBuffer* Buffer, EDataAccessFlag Access);

    void SetShaderImage(const std::string& ParamName, FRDGTexture* Texture, EDataAccessFlag Access);

    void SetShaderSampler(const std::string& ParamName, FRDGTexture* Texture, const RHITextureParams& Sampler=RHITextureParams::DefaultParams);

protected:

    FRDGPassNode* Node;
};

class FRDGComputePassBuilder : public FRDGPassBuilder
{
public:

    FRDGComputePassBuilder();

    void SetComputeShader(const class FShaderPermutationParameters& Params);

};

using PassHandle = int32;

class FRDGBuilder
{
public:
    FRDGBuilder(FDynamicRHI* InRHICmdList) 
        : RHICmdList(InRHICmdList) 
        , TextureAllocator(FRDGTextureAllocator::Get()) 
        , BufferAllocator(FRDGBufferAllocator::Get()) 
        , UniformBufferAllocator(FRDGUniformBufferAllocator::Get()) 
    { }

    FRDGBuilder(const FRDGBuilder&) = delete;

    ~FRDGBuilder() = default;

    FRDGTexture* CreateTexture(const FRDGTextureDesc& TextureDesc);

    FRDGBuffer* CreateBuffer(const FRDGBufferDesc& Desc);

    FRDGUniformBuffer* CreateUniformBuffer(const FRDGUniformBufferDesc& Desc);

    FRDGTexture* RegisterExternalTexture(RHITexture* Texture);

    PassHandle AddComputePass(const std::function<void(FRDGComputePassBuilder&)>& Setup);

private:

    FRenderGraph Graph;

    FRDGTextureAllocator& TextureAllocator;

    FRDGTextureRegistry Textures{};

    FRDGBufferAllocator& BufferAllocator;

    FRDGBufferRegistry Buffers{};

    FRDGUniformBufferAllocator& UniformBufferAllocator;

    FRDGUniformBufferRegistry UniformBuffers{};

    FDynamicRHI* RHICmdList{};

	std::map<RHITexture*, FRDGTexture*> ExternalTextures;
	std::map<RHIBuffer*, FRDGBuffer*> ExternalBuffers;
};

}